/*
 * This file is part of the libopeninv project.
 *
 * Copyright (C) 2018 Johannes Huebner <dev@johanneshuebner.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PIREGULATOR_H
#define PIREGULATOR_H

#include "my_fp.h"
#include "my_math.h"

class PiController
{
   public:
      /** Default constructor */
      PiController();

      /** Set regulator proportional and integral gain.
       * \param kp New value to set for proportional gain
       * \param ki New value for integral gain
       */
      void SetGains(int kp, int ki)
      {
         this->kp = kp;
         this->ki = ki;
      }

      void SetProportionalGain(int kp) { this->kp = kp; }
      void SetIntegralGain(int ki) { this->ki = ki; }

      /** Set regulator target set point
       * \param val regulator target
       */
      void SetRef(s32fp val) { refVal = val; }

      s32fp GetRef() { return refVal; }

      /** Set maximum controller output
        * \param val actuator saturation value
        */
      void SetMinMaxY(int32_t valMin, int32_t valMax) { minY = valMin; maxY = valMax; }

      /** Set calling frequency
       * \param val New value to set
       */
      void SetCallingFrequency(int val) { frequency = val; }

      /** Run controller to obtain a new actuator value
       * \param curVal currently measured value
       * \return new actuator value
       */
      int32_t Run(s32fp curVal);

      /** Run controller to obtain a new actuator value, run only proportional part
       * \param curVal currently measured value
       * \return new actuator value
       */
      int32_t RunProportionalOnly(s32fp curVal);

      /** Reset integrator to 0 */
      void ResetIntegrator() { esum = 0; }

      /** Preload Integrator to yield a certain output
       * @pre SetCallingFrequency() and SetGains() must be called first
      */
      void PreloadIntegrator(int32_t yieldedOutput) { esum = ki != 0 ? FP_FROMINT((yieldedOutput * frequency) / ki) : 0; }

   protected:

   private:
      int32_t kp; //!< Proportional controller gain
      int32_t ki; //!< Integral controller gain
      s32fp esum; //!< Integrator
      s32fp refVal; //!< control target
      int32_t frequency; //!< Calling frequency
      int32_t maxY; //!< upper actuator saturation value
      int32_t minY; //!< lower actuator saturation value
};

#endif // PIREGULATOR_H
