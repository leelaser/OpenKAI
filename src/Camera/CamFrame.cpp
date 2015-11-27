/*
 * CameraInput.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "CamFrame.h"

namespace kai
{

CamFrame::CamFrame()
{
	m_frameID = 0;

	m_iFrame = 0;
	m_pPrev = &m_pFrame[m_iFrame];
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];
}

CamFrame::~CamFrame()
{
}

void CamFrame::getResized(int width, int height, CamFrame* pResult)
{
	if(!pResult)return;
	cv::Size newSize = cv::Size(width,height);

	if(newSize == m_pNext->size())
	{
		pResult->updateFrame(m_pNext);
	}
	else
	{
		cuda::resize(*m_pNext,m_GMat,newSize);
		pResult->updateFrame(&m_GMat);
	}

}

void CamFrame::getGray(CamFrame* pResult)
{
	if(!pResult)return;

	cuda::cvtColor(*m_pNext, *pResult->m_pNext, CV_BGR2GRAY);
}

void CamFrame::getHSV(CamFrame* pResult)
{
	if(!pResult)return;

	//RGB or BGR depends on device
	cuda::cvtColor(*m_pNext, *pResult->m_pNext, CV_BGR2HSV);
}

void CamFrame::getBGRA(CamFrame* pResult)
{
	if(!pResult)return;

	cuda::cvtColor(*m_pNext, *pResult->m_pNext, CV_BGR2BGRA);
}

void CamFrame::get8UC3(CamFrame* pResult)
{
	if(!pResult)return;

	if(m_pNext->type()==CV_8UC3)
	{
		m_pNext->copyTo(*pResult->m_pNext);
	}
	else
	{
		cuda::cvtColor(*m_pNext, *pResult->m_pNext, CV_GRAY2BGR);
	}
}

void CamFrame::switchFrame(void)
{
	//switch the current frame and old frame
	m_pPrev = m_pNext;
	m_iFrame = 1 - m_iFrame;
	m_pNext = &m_pFrame[m_iFrame];
}

void CamFrame::updateFrame(CamFrame* pFrame)
{
	if (pFrame == NULL)return;

	m_frameID = get_time_usec();
	pFrame->getCurrentFrame()->copyTo(*m_pNext);
}


void CamFrame::updateFrame(GpuMat* pGpuFrame)
{
	if (pGpuFrame == NULL)return;

	m_frameID = get_time_usec();
	pGpuFrame->copyTo(*m_pNext);
}

void CamFrame::updateFrame(Mat* pFrame)
{
	if (pFrame == NULL)return;

	m_frameID = get_time_usec();
	m_pNext->upload(*pFrame);
}

GpuMat* CamFrame::getCurrentFrame(void)
{
	return m_pNext;
}

GpuMat* CamFrame::getPreviousFrame(void)
{
	return m_pPrev;
}

uint64_t CamFrame::getFrameID(void)
{
	return m_frameID;
}

bool CamFrame::isNewerThan(CamFrame* pFrame)
{
	if (pFrame == NULL)return false;

	if(pFrame->getFrameID() < m_frameID)
	{
		return true;
	}

	return false;
}


}
