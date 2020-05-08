//	for (row = 0; row < 3; row++)
//		{
//		for (col = 0; col < 3; col++)
//			{
//			for (k = 0; k < 3; k++)
//				{
//				A[row][col] = MultAndAdd(A[row][col], B[row][k], C[k][col]) ;
//				}
//			}
//		}


				LDR	row,=0
TopOfRowLoop:	CMP	row,3
				BGE	RowLoopDone

					LDR	col,=0
	TopOfColLoop:	CMP	col,3
					BGE	ColLoopDone

						LDR	k,=0
		TopOfKLoop:		CMP	k,3
						BGE	KLoopDone

							A[row][col] = MultAndAdd(A[row][col], B[row][k], C[k][col]) ;

						ADD	k,k,1
						B	TopOfKLoop

		KLoopDone:	ADD	col,col,1
					B	TopOfColLoop

ColLoopDone:	ADD	row,row,1
				B	TopOfRowLoop

RowLoopDone:	return

L2:	// continue with other code here
