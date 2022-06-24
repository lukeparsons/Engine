#include "CppUnitTest.h"
#include "../../../Engine/src/math/Matrix.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EngineTests
{
	TEST_CLASS(MathTests)
	{
	public:

		TEST_METHOD(MatrixConstruction)
		{
			const int matrixRow = 3;
			const int matrixColumn = 3;
			Matrixf<matrixRow, matrixColumn> mat;

			for (int i = 0; i < matrixRow; i++)
			{
				for (int j = 0; j < matrixColumn; j++)
				{
					if (mat[i][j] != 0)
					{
						Assert::Fail();
					}
				}
			}
		}
	};
}