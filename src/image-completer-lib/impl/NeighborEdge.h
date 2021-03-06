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

#ifndef NEIGHBOR_EDGE_H
#define NEIGHBOR_EDGE_H

namespace LfnIc
{
	/// Neighbor edges
	enum NeighborEdge
	{
		InvalidNeighborEdge = -1,

		NeighborEdgeLeft,
		NeighborEdgeTop,
		NeighborEdgeRight,
		NeighborEdgeBottom,

		NumNeighborEdges,
		FirstNeighborEdge = NeighborEdgeLeft,
		LastNeighborEdge = NeighborEdgeBottom
	};

	/// Given a valid edge constant, these methods will return -1, 0, or 1 to
	/// indicate the edge's direction.
	bool GetNeighborEdgeDirection(NeighborEdge edge, int& outX, int& outY);
	int GetNeighborEdgeDirectionX(NeighborEdge edge);
	int GetNeighborEdgeDirectionY(NeighborEdge edge);
}

#endif
