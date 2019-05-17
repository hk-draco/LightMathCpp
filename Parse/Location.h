#pragma once

struct Location
{
	int row;
	int column;

	Location() {}
	Location(int row, int column) : row(row), column(column) {}

	static Location closer(Location a, Location b)
	{
		if (a.row < b.row)
			return a;
		else if (a.row == b.row)
			return a.column <= b.column ? a : b;
		else
			return b;
	}

	void write(String* buf) const
	{
		buf->append("[");
		write_int(buf, row);
		buf->append(",");
		write_int(buf, column);
		buf->append("]");
	}

	Location operator +(int length)
	{
		return Location(row, column + length);
	}
};

struct Area
{
	Location begin;
	Location end;

	Area() {}
	Area(Location begin, Location end) : begin(begin), end(end) {}

	void write(String* buf) const
	{
		buf->append("[");
		begin.write(buf);
		buf->append(",");
		end.write(buf);
		buf->append("]");
	}
};