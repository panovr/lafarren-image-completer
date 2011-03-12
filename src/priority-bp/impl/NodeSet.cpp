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
#include "NodeSet.h"

#include "tech/MathUtils.h"

#include "Image.h"
#include "Mask.h"
#include "PriorityBpSettings.h"

#include "tech/DbgMem.h"

namespace PriorityBp
{
	//
	// Helper class for generating the Node set that intersects with the
	// unknown region(s).
	//
	class Lattice
	{
	public:
		Lattice(const Image& inputImage, const MaskLod& mask, Node::Context& nodeContext, std::vector<Node>& nodeStorage);

		void CreateUnknownRegionNodes();
		void ConnectNeighboringNodes();

		int NumCols() const;
		int NumRows() const;

		// May return NULL if no node was created for a lattice point, which will
		// happen if its region does not intersect with the unknown region(s).
		Node* GetNode(int col, int row) const;

	private:
		static const int INVALID_INDEX = -1;

		const Image& m_inputImage;
		const MaskLod& m_mask;
		Node::Context& m_nodeContext;
		std::vector<Node>& m_nodeStorage;
		std::vector<int> m_pointNodeIndices;
		int m_numCols;
		int m_numRows;
	};
}

PriorityBp::Lattice::Lattice(const Image& inputImage, const MaskLod& mask, Node::Context& nodeContext, std::vector<Node>& nodeStorage) :
m_inputImage(inputImage),
m_mask(mask),
m_nodeContext(nodeContext),
m_nodeStorage(nodeStorage),
m_numCols(0),
m_numRows(0)
{
}

void PriorityBp::Lattice::CreateUnknownRegionNodes()
{
	// The lattice has a horizontal and vertical spacing of latticeGapX
	// and latticeGapY, respectively. The nodes will be all lattice
	// points whose patchWidth�patchHeight neighborhood intersects the
	// image's unknown region.
	const int latticeGapX = m_nodeContext.settings.latticeGapX;
	const int latticeGapY = m_nodeContext.settings.latticeGapY;
	const int patchWidth = m_nodeContext.settings.patchWidth;
	const int patchHeight = m_nodeContext.settings.patchHeight;
	const int patchHalfWidth = patchWidth / 2;
	const int patchHalfHeight = patchHeight / 2;

	const int leftMostNodeX = -latticeGapX;
	const int topMostNodeY = -latticeGapY;
	const int rightMostNodeX = m_inputImage.GetWidth() + latticeGapX - 1;
	const int bottomMostNodeY = m_inputImage.GetHeight() + latticeGapY - 1;
	const int nodeSpaceWidth = rightMostNodeX - leftMostNodeX + 1;
	const int nodeSpaceHeight = bottomMostNodeY - topMostNodeY + 1;

	m_numCols = nodeSpaceWidth / latticeGapX;
	m_numRows = nodeSpaceHeight / latticeGapY;
	m_pointNodeIndices.resize(m_numCols * m_numRows);

	for (int pointIndex = 0, row = 0, y = topMostNodeY; row < m_numRows; ++row, y += latticeGapY)
	{
		const int neighborhoodTop = y - patchHalfHeight;
		for (int col = 0, x = leftMostNodeX; col < m_numCols; ++pointIndex, ++col, x += latticeGapX)
		{
			const int neighborhoodLeft = x - patchHalfWidth;
			if (m_mask.RegionXywhHasAny(neighborhoodLeft, neighborhoodTop, patchWidth, patchHeight, Mask::UNKNOWN))
			{
				m_pointNodeIndices[pointIndex] = m_nodeStorage.size();
				m_nodeStorage.push_back(Node(m_nodeContext, m_mask, x, y));
			}
			else
			{
				m_pointNodeIndices[pointIndex] = INVALID_INDEX;
			}
		}
	}
}

void PriorityBp::Lattice::ConnectNeighboringNodes()
{
	for (int row = 0; row < m_numRows; ++row)
	{
		for (int col = 0; col < m_numCols; ++col)
		{
			Node* node = GetNode(col, row);
			if (node)
			{
				for (int edgeIdx = FirstNeighborEdge; edgeIdx <= LastNeighborEdge; ++edgeIdx)
				{
					const NeighborEdge edge = NeighborEdge(edgeIdx);
					int edgeDirectionX;
					int edgeDirectionY;
					GetNeighborEdgeDirection(edge, edgeDirectionX, edgeDirectionY);

					const int neighborCol = col + edgeDirectionX;
					const int neighborRow = row + edgeDirectionY;

					Node* neighbor = GetNode(neighborCol, neighborRow);
					if (neighbor)
					{
						node->AddNeighbor(*neighbor, edge);
					}
				}
			}
		}
	}
}

int PriorityBp::Lattice::NumCols() const
{
	return m_numCols;
}

int PriorityBp::Lattice::NumRows() const
{
	return m_numRows;
}

PriorityBp::Node* PriorityBp::Lattice::GetNode(int col, int row) const
{
	Node* node = NULL;

	if (col >= 0 && row >= 0 && col < m_numCols && row < m_numRows)
	{
		const int pointIndex = Tech::GetRowMajorIndex(m_numCols, col, row);
		wxASSERT(pointIndex >= 0 && pointIndex < int(m_pointNodeIndices.size()));

		const int nodeIndex = m_pointNodeIndices[pointIndex];
		if (nodeIndex >= 0 && nodeIndex < int(m_nodeStorage.size()))
		{
			node = &m_nodeStorage[nodeIndex];
		}
	}

	return node;
}

//
// NodeSet implementation
//
PriorityBp::NodeSet::NodeSet(
	const Settings& settings,
	const Image& inputImage,
	const MaskLod& mask,
	const LabelSet& labelSet,
	EnergyCalculatorContainer& energyCalculatorContainer) :
m_nodeContext(settings, labelSet, energyCalculatorContainer),
m_depth(0)
{
	Lattice lattice(inputImage, mask, m_nodeContext, *this);
	lattice.CreateUnknownRegionNodes();
	lattice.ConnectNeighboringNodes();

	m_nodeSetInfo.resize(size());
}

void PriorityBp::NodeSet::UpdatePriority(const Node& node)
{
	for (int i = 0, n = size(); i < n; ++i)
	{
		if (&(operator[](i)) == &node)
		{
			m_nodeSetInfo[i].priority = node.CalculatePriority();
			break;
		}
	}
}

PriorityBp::Priority PriorityBp::NodeSet::GetPriority(const Node& node) const
{
	Priority priority = PRIORITY_MIN;
	for (int i = 0, n = size(); i < n; ++i)
	{
		if (&(operator[](i)) == &node)
		{
			priority = m_nodeSetInfo[i].priority;
			break;
		}
	}

	return priority;
}

void PriorityBp::NodeSet::SetCommitted(const Node& node, bool committed)
{
	for (int i = 0, n = size(); i < n; ++i)
	{
		if (&(operator[](i)) == &node)
		{
			m_nodeSetInfo[i].isCommitted = committed;
			break;
		}
	}
}

bool PriorityBp::NodeSet::IsCommitted(const Node& node) const
{
	for (int i = 0, n = size(); i < n; ++i)
	{
		if (&(operator[](i)) == &node)
		{
			return m_nodeSetInfo[i].isCommitted;
		}
	}

	return false;
}

PriorityBp::Node* PriorityBp::NodeSet::GetHighestPriorityUncommittedNode() const
{
	Node* node = NULL;
	Priority priorityHighest = PRIORITY_MIN;
	for (int i = 0, n = size(); i < n; ++i)
	{
		if (!m_nodeSetInfo[i].isCommitted && m_nodeSetInfo[i].priority > priorityHighest)
		{
			node = const_cast<Node*>(&at(i));
			priorityHighest = m_nodeSetInfo[i].priority;
		}
	}

	return node;
}

void PriorityBp::NodeSet::ScaleUp()
{
	wxASSERT(m_depth > 0);
	--m_depth;

	for (int i = 0, n = size(); i < n; ++i)
	{
		Node& node = at(i);
		node.ScaleUp();
	}
}

void PriorityBp::NodeSet::ScaleDown()
{
	wxASSERT(m_depth >= 0);
	++m_depth;

	for (int i = 0, n = size(); i < n; ++i)
	{
		Node& node = at(i);
		node.ScaleDown();
	}
}

int PriorityBp::NodeSet::GetScaleDepth() const
{
	return m_depth;
}

PriorityBp::NodeSet::NodeInfo::NodeInfo() :
priority(PRIORITY_MIN),
isCommitted(false)
{
}
