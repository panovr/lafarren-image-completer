//
// Copyright 2010, Darren Lafreniere
// <http://www.lafarren.com/image-completer/>
// 
// This file is part of lafarren.com's Image Completer.
// 
// Image Completer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Image Completer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Image Completer, named License.txt. If not, see
// <http://www.gnu.org/licenses/>.
//

#include "Pch.h"
#include "HostImageLocal.h"

PriorityBp::HostImageLocal::HostImageLocal()
	: m_width(0)
	, m_height(0)
	, m_rgb(NULL)
{
}

PriorityBp::HostImageLocal::HostImageLocal(int width, int height)
	: m_width(0)
	, m_height(0)
	, m_rgb(NULL)
{
	InitInternal(width, height);
}

PriorityBp::HostImageLocal::~HostImageLocal()
{
	delete [] m_rgb;
}

bool PriorityBp::HostImageLocal::Init(int width, int height)
{
	return InitInternal(width, height);
}

bool PriorityBp::HostImageLocal::IsValid() const
{
	return IsValidInternal();
}

PriorityBp::HostImage::Rgb* PriorityBp::HostImageLocal::GetRgb()
{
	return m_rgb;
}

const PriorityBp::HostImage::Rgb* PriorityBp::HostImageLocal::GetRgb() const
{
	return m_rgb;
}

int PriorityBp::HostImageLocal::GetWidth() const
{
	return m_width;
}

int PriorityBp::HostImageLocal::GetHeight() const
{
	return m_height;
}

bool PriorityBp::HostImageLocal::InitInternal(int width, int height)
{
	delete [] m_rgb;
	m_rgb = NULL;

	m_width = std::max(width, 0);
	m_height = std::max(height, 0);
	const int numPixels = width * height;
	if (numPixels > 0)
	{
		m_rgb = new Rgb[numPixels];
	}

	return IsValidInternal();
}

bool PriorityBp::HostImageLocal::IsValidInternal() const
{
	return m_width > 0 && m_height > 0 && m_rgb;
}
