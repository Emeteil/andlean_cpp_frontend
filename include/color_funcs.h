/*
 * Copyright 2011 Maciej Ciemborowicz
 * https://maciej-ciemborowicz.eu
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

typedef struct rgb {
  float r, g, b;
} RGB;

typedef struct hsl {
  float h, s, l;
} HSL;;

HSL rgb2hsl(float r, float g, float b);
float hue2rgb(float p, float q, float t);
RGB hsl2rgb(float h, float s, float l);