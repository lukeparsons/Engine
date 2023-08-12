#pragma once

#define DEF_STRING "Error in accessing value in Either type"
#define DEF_INT 0

template<typename A, typename B>
class Either
{
protected:

	Either<A, B>() : is_left(true) {};

	union
	{
		A left;
		B right = B();
	};

	bool is_left;

public:

	~Either<A, B>()
	{
		if (is_left)
		{
			left.~A();
		} else
		{
			right.~B();
		}
	}

	Either<A, B>(const Either<A, B>& either)
	{
		is_left = either.is_left;

		is_left ? left = either.left : right = either.right;
	}

	A fromLeft(A def) const
	{
		return is_left ? left : def;
	}

	B fromRight(B def) const
	{
		return !is_left ? right : def;
	}

	bool isLeft()
	{
		return is_left;
	}

	bool isRight()
	{
		return !is_left;
	}
};

template<typename A, typename B>
struct Left : public Either<A, B>
{
	Left(A left)
	{
		this->is_left = false;
		this->left = left;
	}
};

template<typename A, typename B>
struct Right : public Either<A, B>
{
	Right(B right)
	{
		this->is_left = false;
		this->right = right;
	}
};