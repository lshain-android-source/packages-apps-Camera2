/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* $Id: db_image_homography.h,v 1.2 2011/06/17 14:03:31 mbansal Exp $ */

#ifndef DB_IMAGE_HOMOGRAPHY
#define DB_IMAGE_HOMOGRAPHY



/*****************************************************************
*    Lean and mean begins here                                   *
*****************************************************************/

#include "db_framestitching.h"
/*!
 * \defgroup LMImageHomography (LM) Image Homography Estimation (feature based)
 */
/*\{*/
/*!
Solve for projective H such that xp~Hx. Prior normalization is not necessary,
although desirable for numerical conditioning
\param H    image projective (out)
\param x1   image 1 point 1
\param x2   image 1 point 2
\param x3   image 1 point 3
\param x4   image 1 point 4
\param xp1  image 2 point 1
\param xp2  image 2 point 2
\param xp3  image 2 point 3
\param xp4  image 2 point 4
*/
DB_API void db_StitchProjective2D_4Points(double H[9],
                                      double x1[3],double x2[3],double x3[3],double x4[3],
                                      double xp1[3],double xp2[3],double xp3[3],double xp4[3]);

/*!
Solve for affine H such that xp~Hx. Prior normalization is not necessary,
although desirable for numerical conditioning
\param H    image projective (out)
\param x1   image 1 point 1
\param x2   image 1 point 2
\param x3   image 1 point 3
\param xp1  image 2 point 1
\param xp2  image 2 point 2
\param xp3  image 2 point 3
*/
DB_API void db_StitchAffine2D_3Points(double H[9],
                                      double x1[3],double x2[3],double x3[3],
                                      double xp1[3],double xp2[3],double xp3[3]);

/*!
Solve for rotation R such that xp~Rx.
Image points have to be of unit norm for the least squares to be meaningful.
\param R    image rotation (out)
\param x1   image 1 point 1
\param x2   image 1 point 2
\param xp1  image 2 point 1
\param xp2  image 2 point 2
*/
inline void db_StitchCameraRotation_2Points(double R[9],
                                            /*Image points have to be of unit norm
                                            for the least squares to be meaningful*/
                                            double x1[3],double x2[3],
                                            double xp1[3],double xp2[3])
{
    double* x[2];
    double* xp[2];
    double scale,t[3];

    x[0]=x1;
    x[1]=x2;
    xp[0]=xp1;
    xp[1]=xp2;
    db_StitchSimilarity3DRaw(&scale,R,t,xp,x,2,1,0,1,0);
}

/*!
Solve for a homography H generated by a rotation R with a common unknown focal length f, i.e.
H=diag(f,f,1)*R*diag(1/f,1/f,1) such that xp~Hx.
If signed_disambiguation is true, the points are
required to be in front of the camera. No specific normalization of the homogenous points
is required, although it could be desirable to keep x1,x2,xp1 and xp2 of reasonable magnitude.
If a solution is obtained the function returns 1, otherwise 0. If the focal length is desired
a valid pointer should be passed in f
*/
DB_API int db_StitchRotationCommonFocalLength_3Points(double H[9],double x1[3],double x2[3],double x3[3],
                                                      double xp1[3],double xp2[3],double xp3[3],double *f=0,int signed_disambiguation=1);

/*!
Find scale, rotation and translation of the similarity that
takes the nr_points inhomogenous 2D points X to Xp,
i.e. for the homogenous equivalents
Xp and X we would have
\code
Xp~
[sR t]*X
[0  1]
\endcode
If orientation_preserving is true, R is restricted such that det(R)>0.
allow_scaling, allow_rotation and allow_translation allow s,R and t
to differ from 1,Identity and 0

Full similarity takes the following on 550MHz:
\code
0.9 microseconds with       2 points
1.0 microseconds with       3 points
1.1 microseconds with       4 points
1.3 microseconds with       5 points
1.4 microseconds with       6 points
1.7 microseconds with      10 points
9   microseconds with     100 points
130 microseconds with    1000 points
1.3 milliseconds with   10000 points
35  milliseconds with  100000 points
350 milliseconds with 1000000 points
\endcode

Without orientation_preserving:
\code
3 points is minimal for (s,R,t) (R,t)
2 points is minimal for (s,t) (s,R) (R)
1 point is minimal for  (s) (t)
\endcode

With orientation_preserving:
\code
2 points is minimal for (s,R,t) (R,t) (s,t)
1 point is minimal for (s,R) (R) (s) (t)
\endcode
\param scale        (out)
\param R            2D rotation (out)
\param t            2D translation (out)
\param Xp           (nr_points x 2) pointer to array of image points
\param X            (nr_points x 2 ) pointer to array of image points
\param nr_points    number of points
\param orientation_preserving
\param allow_scaling    compute scale (if 0, scale=1)
\param allow_rotation   compute rotation (if 0, R=[I])
\param allow_translation compute translation (if 0 t = [0,0]')
*/
DB_API void db_StitchSimilarity2DRaw(double *scale,double R[4],double t[2],
                            double **Xp,double **X,int nr_points,int orientation_preserving=1,
                            int allow_scaling=1,int allow_rotation=1,int allow_translation=1);
/*!
See db_StitchRotationCommonFocalLength_3Points().
\param H            Image similarity transformation (out)
\param Xp           (nr_points x 2) pointer to array of image points
\param X            (nr_points x 2) pointer to array of image points
\param nr_points    number of points
\param orientation_preserving
\param allow_scaling    compute scale (if 0, scale=1)
\param allow_rotation   compute rotation (if 0, R=[I])
\param allow_translation compute translation (if 0 t = [0,0]')
*/
inline void db_StitchSimilarity2D(double H[9],double **Xp,double **X,int nr_points,int orientation_preserving=1,
                                  int allow_scaling=1,int allow_rotation=1,int allow_translation=1)
{
    double s,R[4],t[2];

    db_StitchSimilarity2DRaw(&s,R,t,Xp,X,nr_points,orientation_preserving,
        allow_scaling,allow_rotation,allow_translation);

    H[0]=s*R[0]; H[1]=s*R[1]; H[2]=t[0];
    H[3]=s*R[2]; H[4]=s*R[3]; H[5]=t[1];
    db_Zero2(H+6);
    H[8]=1.0;
}
/*\}*/
#endif /* DB_IMAGE_HOMOGRAPHY */
