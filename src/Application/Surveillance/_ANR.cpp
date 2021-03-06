/*
 * _ANR.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_ANR.h"

namespace kai
{

_ANR::_ANR()
{
	m_pDcn = NULL;
	m_pDlp = NULL;
	m_cnPrefix = "";
	m_cnPosMargin = 0.05;
	m_cn = "";
	m_cnPrefixBB.init();
	m_cnBB.init();
	m_tStampCNprefix = 0;
	m_tStampCN = 0;
	m_timeOut = USEC_1SEC;
	m_nCNdigit = 6;
	m_offsetRdigit = 0.01;
	m_wRdigit = 0.05;

	m_lp = "";
	m_lpBB.init();
	m_tStampLP = 0;

	m_tempFile = "anr.jpg";
	m_shFile = "alpr.sh";
	m_vAlprROI.init();
	m_vAlprROI.z = 1.0;
	m_vAlprROI.w = 1.0;
	m_pWS = NULL;

#ifdef USE_OCR
	m_bOCR = false;
	m_pOCR = NULL;
#endif

}

_ANR::~_ANR()
{
}

bool _ANR::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("cnPrefix",&m_cnPrefix);
	pK->v("cnPosMargin",&m_cnPosMargin);
	pK->v("nCNdigit",&m_nCNdigit);
	pK->v("offsetRdigit",&m_offsetRdigit);
	pK->v("wRdigit",&m_wRdigit);

	Kiss* pA = pK->o("oalpr");
	if(pA)
	{
		int jpgQuality = 80;
		pA->v("jpgQuality",&jpgQuality);
		m_vJPGquality.push_back(IMWRITE_JPEG_QUALITY);
		m_vJPGquality.push_back(jpgQuality);
		pA->v("x",&m_vAlprROI.x);
		pA->v("y",&m_vAlprROI.y);
		pA->v("z",&m_vAlprROI.z);
		pA->v("w",&m_vAlprROI.w);
		pA->v("tempFile",&m_tempFile);
		pA->v("shFile",&m_shFile);
	}

#ifdef USE_OCR
	KISSm(pK,bOCR);
#endif

	if(pK->v("timeOut",&m_timeOut))
		m_timeOut *= USEC_1SEC;

	vector<string> vPrefix;
	pK->a("vPrefix", &vPrefix);
	for(int i=0; i<vPrefix.size(); i++)
		m_vPrefixCandidate.push_back(vPrefix[i]);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_DetectorBaseCN", &iName));
	m_pDcn = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDcn, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_DetectorBaseLP", &iName));
	m_pDlp = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDlp, iName + " not found");

	iName = "";
	F_INFO(pK->v("_WebSocket", &iName));
	m_pWS = (_WebSocket*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pWS, iName + " not found");

#ifdef USE_OCR
	IF_T(!m_bOCR);
	iName = "";
	F_INFO(pK->v("OCR", &iName));
	m_pOCR = (OCR*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pOCR, iName + " not found");
#endif

	return true;
}

bool _ANR::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _ANR::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if(cn())
		{
			lpO();
		}

//		lp();

		this->autoFPSto();
	}
}

int _ANR::check(void)
{
	NULL__(m_pDcn,-1);
	NULL__(m_pDlp,-1);
	NULL__(m_pWS,-1);

	return 0;
}

string _ANR::char2Number(const char* pStr)
{
	string asc = "";
	if(!pStr)return asc;

	int i=0;
	while(pStr[i]!=0)
	{
		char c = pStr[i++];

		if(c=='O' ||
		   c=='o' ||
		   c=='D' ||
		   c=='Q' ||
		   c=='C')
			c='0';

		if(c=='I' ||
		   c=='L' ||
		   c=='J')
			c='1';

		if(c=='Z')
			c='2';

		asc += c;
	}

	return asc;
}

bool _ANR::cn(void)
{
	IF__(check()<0, false);

	OBJECT* pO;
	int i = 0;
	while ((pO = m_pDcn->at(i++)) != NULL)
	{
		vFloat4 bb = pO->m_bb;
		string s = string(pO->m_pText);
		s = deleteNonASCII(s.c_str());

		bool bMatch = false;
		for(int j=0; j<m_vPrefixCandidate.size(); j++)
		{
			std::string::size_type pos = s.find(m_vPrefixCandidate[j]);
			IF_CONT(pos == std::string::npos);

			bMatch = true;
			break;
		}

		if(bMatch)
		{
			m_cnPrefixBB = bb;
			m_tStampCNprefix = getTimeUsec();
			break;
		}
	}

	if(getTimeUsec() - m_tStampCNprefix > m_timeOut)
	{
		m_cn = "";
		return false;
	}

	string cn;
	i=0;
	while ((pO = m_pDcn->at(i++)) != NULL)
	{
		vFloat4 bb = pO->m_bb;

		IF_CONT(bb.x < m_cnPrefixBB.x);
		IF_CONT(abs(bb.y - m_cnPrefixBB.y) > m_cnPosMargin);
		IF_CONT(abs(bb.w - m_cnPrefixBB.w) > m_cnPosMargin);

		string s = string(pO->m_pText);
		s = char2Number(s.c_str());
		s = deleteNonNumber(s.c_str());
		IF_CONT(s.length() < m_nCNdigit);

		cn = m_cnPrefix + s.substr(0,m_nCNdigit);
		m_cnBB = bb;
		m_tStampCN = getTimeUsec();
	}

	if(getTimeUsec() - m_tStampCN > m_timeOut)
	{
		m_cn = "";
		return false;
	}

	IF__(cn == m_cn, false);

	m_cn = cn;
	return true;
}

void _ANR::lpO(void)
{
	IF_(check() < 0);
	IF_(m_cn.empty());

	NULL_(m_pDlp->m_pV);
	IF_(m_pDlp->m_pV->BGR()->bEmpty());
	m_fBGR.copy(*m_pDlp->m_pV->BGR());
	cv::imwrite(m_tempFile, *m_fBGR.m(), m_vJPGquality);

	FILE *fp;
	char path[1035];

	fp = popen(m_shFile.c_str(), "r");
	NULL_l(fp,"Failed to run command:" + m_shFile);

	while (fgets(path, sizeof(path) - 1, fp));
	pclose(fp);

	string strR = string(path);
	std::string::size_type k;

	k = strR.find("\\u");
	while (k != std::string::npos)
	{
		strR.replace(k,6,"-");
		k = strR.find("\\u");
	}

	k = strR.find("\"");
	while (k != std::string::npos)
	{
		strR.erase(k,1);
		k = strR.find("\"");
	}

	k = strR.find(":");
	while (k != std::string::npos)
	{
		strR.erase(k,1);
		k = strR.find(":");
	}

	k = strR.find(" plate");
	if(k == std::string::npos)
		return;

	strR.erase(k,7);
	std::string::size_type m;
	m = strR.find(',', k);

	string cStr = strR.substr(k, m-k);
	cStr += " " + m_cn;

	m_pWS->write((unsigned char*)cStr.c_str(),
			cStr.length(),
			WS_MODE_TXT);

	if (getTimeUsec() - m_tStampLP > m_timeOut)
	{
		m_lp = "";
		return;
	}

}

void _ANR::lp(void)
{
	IF_(check()<0);

#ifdef USE_OCR
	if(m_bOCR && m_pOCR)
	{
		vInt2 cs;
		cs.x = m_pDlp->m_fBGR.m()->cols;
		cs.y = m_pDlp->m_fBGR.m()->rows;

		m_lpBB.x = m_cnPrefixBB.x - m_cnPrefixBB.width() * 1.0;
		m_lpBB.y = m_cnPrefixBB.y + m_cnPrefixBB.height() * 18;
		m_lpBB.z = m_lpBB.x + m_cnPrefixBB.width() * 2.0;
		m_lpBB.w = m_lpBB.y + m_cnPrefixBB.height() * 3.5;

		m_lpBB.constrain(0.0,1.0);

		Rect r;
		r.x = m_lpBB.x * cs.x;
		r.width = (m_lpBB.z - m_lpBB.x) * cs.x;
		r.y = m_lpBB.y * cs.y;
		r.height = (m_lpBB.w - m_lpBB.y) * cs.y;

		m_pOCR->setFrame(m_pDlp->m_fBGR);
		m_lp = m_pOCR->scan(&r);
	}
	else
	{
		OBJECT* pO;
		int i = 0;
		while ((pO = m_pDlp->at(i++)) != NULL)
		{
			string s = string(pO->m_pText);
//			IF_CONT(s.length() < 1);
//			IF_CONT(s.length() > 4);
			s = char2Number(s.c_str());
			s = deleteNonNumber(s.c_str());

			m_lp = s.substr(0,4);
			m_lpBB = pO->m_bb;

			if(m_bOCR && m_pOCR)
			{
				vInt2 cs;
				cs.x = m_pDlp->m_fBGR.m()->cols;
				cs.y = m_pDlp->m_fBGR.m()->rows;
				Rect r = convertBB<vInt4>(convertBB(pO->m_bb, cs));

				m_pOCR->setFrame(m_pDlp->m_fBGR);
				m_lp = m_pOCR->scan(&r);
			}

			m_tStampLP = getTimeUsec();
		}
	}
#endif

	LOG_I("L Number: " + m_lp);

	if(getTimeUsec() - m_tStampLP > m_timeOut)
	{
		m_lp = "";
		return;
	}
}

void _ANR::draw(void)
{
	this->_ThreadBase::draw();

	if(getTimeUsec() - m_tStampCN > m_timeOut)
		addMsg("CN unrecognized");
	else
		addMsg("CN: " + m_cn);

	IF_(!checkWindow());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	IF_(getTimeUsec() - m_tStampCN > m_timeOut);

	Scalar col = Scalar(0,0,255);

	putText(*pMat, string(m_cn),
			Point(10, pMat->rows*0.8),
			FONT_HERSHEY_SIMPLEX, 3.0, col, 5);

	Rect r;
	r.x = m_cnPrefixBB.x * pMat->cols;
	r.y = m_cnPrefixBB.y * pMat->rows;
	r.width = m_cnBB.z * pMat->cols - r.x;
	r.height = m_cnBB.w * pMat->rows - r.y;
	rectangle(*pMat, r, col, 2);

	putText(*pMat, string(m_lp),
			Point(10, pMat->rows*0.9),
			FONT_HERSHEY_SIMPLEX, 3.0, col, 5);

	r.x = m_lpBB.x * pMat->cols;
	r.y = m_lpBB.y * pMat->rows;
	r.width = m_lpBB.z * pMat->cols - r.x;
	r.height = m_lpBB.w * pMat->rows - r.y;
	rectangle(*pMat, r, col, 2);

}

}
