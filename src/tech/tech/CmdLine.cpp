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
#include "tech/CmdLine.h"

#include "tech/DbgMem.h"

using namespace Lafarren;

CmdLine::Param::Param(const wxString& shortName, const wxString& longName, const wxString description)
	: shortName(shortName)
	, longName(longName)
	, description(description)
	, isSet(false)
{
}

void CmdLine::Param::Set(){
	isSet = true;
}

void CmdLine::AddParam(Param& param)
{
	bool add = true;

	for (int i = 0, n = m_params.size(); i < n; ++i)
	{
		const bool isDuplicate = (m_params[i]->shortName == param.shortName || m_params[i]->shortName == param.longName);
		wxASSERT(!isDuplicate);
		if (isDuplicate)
		{
			add = false;
			break;
		}
	}

	if (add)
	{
		m_params.push_back(&param);
	}
}

bool CmdLine::Read(int argc, char** argv, wxString& outError) const
{
	outError.Empty();

	for (int i = 1; i < argc; ++i)
	{
		const char* arg = argv[i];
		Param* param = GetParam(arg);

		if (!param)
		{
			outError = wxString::Format("Invalid argument: %s\n", arg);
			break;
		}
		else
		{
			param->Set();
			if (param->IsOption())
			{
				++i;
				wxASSERT(i <= argc);
				if (i == argc)
				{
					outError = wxString::Format("Argument was missing value: %s\n", arg);
					break;
				}
				else
				{
					param->ReadOption(argv[i]);
				}
			}
		}
	}

	return outError.IsEmpty();
}

wxString CmdLine::GetUsageString() const
{
	wxString usage("\nOptions:\n\n");
	
	const wxString indent("  ");
	for (int i = 0, n = m_params.size(); i < n; ++i)
	{
		const Param& param = *m_params[i];
		usage += indent + param.shortName + ", " + param.longName;
		if (param.IsOption())
		{
			usage += " <value>";
		}

		usage += "\n";
		usage += indent;

		wxString description(param.description);
		description.Replace("\t", indent);
		usage += description;
		usage += "\n\n";
	}

	return usage;
}

CmdLine::Param* CmdLine::GetParam(const char* arg) const
{
	Param* param = NULL;
	for (int i = 0, n = m_params.size(); i < n; ++i)
	{
		if (m_params[i]->shortName == arg || m_params[i]->longName == arg)
		{
			param = m_params[i];
			break;
		}
	}

	return param;
}