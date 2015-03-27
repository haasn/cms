module MatrixGen
  ( module Data.Packed.Matrix
  , module Numeric.LinearAlgebra.Algorithms
  , module Numeric.Container
  , module MatrixGen
  ) where

import Data.Packed.Matrix
import Numeric.LinearAlgebra.Algorithms
import Numeric.Container

type XYZ = (Double, Double, Double)
type Yxy = (Double, Double, Double)

type Prim  = (XYZ, XYZ, XYZ)
type Space = (Prim, XYZ)

_YxytoXYZ :: Yxy -> XYZ
_YxytoXYZ (_Y,x,y) = (x/y, _Y, (1-x-y)/y)

xytoXYZ :: Double -> Double -> XYZ
xytoXYZ x y = _YxytoXYZ (1,x,y)

d65 :: XYZ
d65 = xytoXYZ 0.3127 0.3290

bt709 :: Prim
bt709 = ( xytoXYZ 0.640 0.330
        , xytoXYZ 0.300 0.600
        , xytoXYZ 0.150 0.060
        )

bt2020 :: Prim
bt2020 = ( xytoXYZ 0.708 0.292
         , xytoXYZ 0.170 0.797
         , xytoXYZ 0.131 0.046
         )

bt601_525, bt601_625 :: Prim
bt601_525 = ( xytoXYZ 0.630 0.340
            , xytoXYZ 0.310 0.595
            , xytoXYZ 0.155 0.070
            )

bt601_625 = ( xytoXYZ 0.640 0.330
            , xytoXYZ 0.290 0.600
            , xytoXYZ 0.150 0.060
            )

_M :: Space -> Matrix Double
_M ((r, g, b), w) = (3><3) [ _Sr*_Xr, _Sg*_Xg, _Sb*_Xb
                           , _Sr*_Yr, _Sg*_Yg, _Sb*_Yb
                           , _Sr*_Zr, _Sg*_Zg, _Sb*_Zb
                           ]
  where (_Xr,_Yr,_Zr) = r
        (_Xg,_Yg,_Zg) = g
        (_Xb,_Yb,_Zb) = b
        (_Xw,_Yw,_Zw) = w

        [_Sr,_Sg,_Sb] = concat . toLists $ inv _XYZ <> (3><1) [_Xw,_Yw,_Zw]
        _XYZ = (3><3) [ _Xr, _Xg, _Xb
                      , _Yr, _Yg, _Yb
                      , _Zr, _Zg, _Zb
                      ]

{-
main = do
  let to2020  = inv from2020
      to709   = inv from709
      from709  = _M (bt709, d65)
      from2020 = _M (bt2020, d65)
      from525  = _M (bt601_525, d65)
      from625  = _M (bt601_625, d65)

  putStrLn "709 → 2020"
  print (to2020 <> from709)

  putStrLn "2020 → 709"
  print (to709 <> from2020)

  putStrLn "525 → 2020"
  print (to2020 <> from525)

  putStrLn "625 → 2020"
  print (to2020 <> from625)
-}
