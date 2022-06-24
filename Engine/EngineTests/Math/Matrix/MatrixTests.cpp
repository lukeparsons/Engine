#include "CppUnitTest.h"
#include "../../../Engine/src/math/Matrix.h"
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EngineTests
{
	TEST_CLASS(MathTests)
	{
	private:
		template<size_t row, size_t column>
		bool areAllZero(Matrixf<row, column> mat)
		{
			for (int i = 0; i < row; i++)
			{
				for (int j = 0; j < column; j++)
				{
					if (mat[i][j] != 0)
					{
						return false;
					}
				}
			}
			return true;
		}
	public:
		
		/// <summary>
		/// Tests constructing and initalising a matrix to all 0 as well as the overloaded index [] operator
		/// </summary>
		TEST_METHOD(MatrixConstructionAndIndex)
		{
			const int matrixRow = 3;
			const int matrixColumn = 4;
			Matrixf<matrixRow, matrixColumn> mat;

			Assert::IsTrue(areAllZero(mat));
		}

		/// <summary>
		/// Tests that we get an all zero matrix of the correct dimensions when we attempt to multiply two matrices 
		/// where the number of columns of the 1st matrix is not equal to the number of rows of the second matrix
		/// </summary>
		TEST_METHOD(MatrixInvalidMultiply)
		{
			Matrixf<2, 2> mat1;
			Matrixf<3, 1> mat2;

			Matrixf<2, 1> result = mat1 * mat2;

			Assert::IsTrue(areAllZero(result));
		}
		
		/// <summary>
		/// Tests that two matrices (non-square) multiplied together give the correct result
		/// </summary>
		TEST_METHOD(MatrixMultiply)
		{
			/* 1 2 3
			   4 5 6 */
			Matrixf<2, 3> mat1;
			mat1[0][0] = 1;
			mat1[0][1] = 2;
			mat1[0][2] = 3;
			mat1[1][0] = 4;
			mat1[1][1] = 5;
			mat1[1][2] = 6;

			/* 2
			   4
			   8 */
			Matrixf<3, 1> mat2;
			mat2[0][0] = 2;
			mat2[1][0] = 4;
			mat2[1][1] = 8;

			/* Should produce:
			* 34
			  76 */
			Matrixf<2, 1> result = mat1 * mat2;

			Assert::AreEqual(34.0f, result[0][0]);
			Assert::AreEqual(76.0f, result[1][0]);
		}
	};


}