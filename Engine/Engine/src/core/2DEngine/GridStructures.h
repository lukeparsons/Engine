#pragma once
#include <algorithm>

struct CellLocation
{
	unsigned int i, j;

	CellLocation(unsigned int _i, unsigned int _j) : i(_i), j(_j) {};

	bool operator<(const CellLocation& rhs) const
	{
		if(i == rhs.i)
		{
			return j < rhs.j;
		} if(j == rhs.j)
		{
			return i < rhs.i;
		} else
		{
			return i < rhs.i;
		}
	}
};

template<typename T, size_t n>
class RowVector
{
private:
	Matrix<T, n, 1> vector;

	T defaultValue;
public:

	RowVector(T _defaultValue) : defaultValue(_defaultValue) {};

	RowVector(const Matrix<T, n, 1>&& matrix, T _defaultValue) : vector(matrix), defaultValue(_defaultValue) {};
	
	RowVector(const RowVector& copyVector) : vector(copyVector.vector), defaultValue(copyVector.defaultValue) {};

	RowVector(RowVector&& moveVector) : vector(moveVector.vector), defaultValue(moveVector.defaultValue) {};

	RowVector& operator=(const RowVector& other)
	{
		vector = other.vector;
		defaultValue = other.defaultValue;
		return *this;
	}

	Matrix<T, n, 1>& GetMatrix()
	{
		return vector;
	}

	const Matrix<T, n, 1>& GetMatrix() const
	{
		return vector;
	}

	T GetDefaultValue() const
	{
		return defaultValue;
	}

	T& operator[](const size_t row)
	{
		return row >= 0 && row <= n ? vector[row][0] : defaultValue;
	}

};

template<typename T, size_t n>
T DotProduct(const RowVector<T, n>& lhs, const RowVector<T, n>& rhs)
{
	return DotProduct(lhs.GetMatrix(), rhs.GetMatrix());
}

template<typename T, size_t n>
RowVector<T, n> operator*(T scalar, RowVector<T, n>& lhs)
{
	return RowVector(scalar * lhs.GetMatrix(), lhs.GetDefaultValue());
}

template<typename T, size_t n>
RowVector<T, n> operator*(const RowVector<T, n>& lhs, const RowVector<T, n>& rhs)
{
	return RowVector(lhs.GetMatrix() * rhs.GetMatrix(), lhs.GetDefaultValue());
}

template<typename T, size_t n>
RowVector<T, n> operator-(const RowVector<T, n>& lhs, const RowVector<T, n>& rhs)
{
	return RowVector(lhs.GetMatrix() - rhs.GetMatrix(), lhs.GetDefaultValue());
}

template<typename T, size_t n>
RowVector<T, n> operator+(const RowVector<T, n>& lhs, const RowVector<T, n>& rhs)
{
	return RowVector(lhs.GetMatrix() + rhs.GetMatrix(), lhs.GetDefaultValue());
}

template<typename T, size_t n>
T max(const RowVector<T, n>& vector)
{
	return *std::max_element(vector.GetMatrix()[0], vector.GetMatrix()[0] + n);
}

template<typename Key, typename Value>
class GridDataMap
{
private:
	std::map<Key, Value> map;
	Value defaultValue;
public:

	GridDataMap(const Value& _defaultValue) : defaultValue(_defaultValue) {};

	Value& operator[](const Key& key)
	{
		typename std::map<Key, Value>::iterator it = map.find(key);
		return it != map.end() ? it->second : defaultValue;
	}

	Value operator[](const Key& key) const
	{
		return this[key];
	}

	void insert(const Key& key, const Value& value)
	{
		map[key] = value;
	}

	void insert(const Key& key, const Value&& value)
	{
		map.emplace(key, std::move(value));
	}

	std::map<Key, Value>::iterator begin()
	{
		return map.begin();
	}

	std::map<Key, Value>::iterator end()
	{
		return map.end();
	}

	std::map<Key, Value>::reverse_iterator rbegin()
	{
		return map.rbegin();
	}

	std::map<Key, Value>::reverse_iterator rend()
	{
		return map.rend();
	}

};