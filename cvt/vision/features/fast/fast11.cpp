#include <cvt/vision/features/FAST.h>

namespace cvt {

	int FAST::score11Pixel( const uint8_t* p, const int * offsets, uint8_t threshold )
	{
		int bmin = threshold;
		int bmax = 255;
		int b = (bmax + bmin)/2;

		/*Compute the score using binary search*/
		for(;;)
		{
			int cb = *p + b;
			int c_b= *p - b;


			if( p[offsets[0]] > cb)
				if( p[offsets[1]] > cb)
					if( p[offsets[2]] > cb)
						if( p[offsets[3]] > cb)
							if( p[offsets[4]] > cb)
								if( p[offsets[5]] > cb)
									if( p[offsets[6]] > cb)
										if( p[offsets[7]] > cb)
											if( p[offsets[8]] > cb)
												if( p[offsets[9]] > cb)
													if( p[offsets[10]] > cb)
														goto is_a_corner;
													else
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
										else
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
									else
										if( p[offsets[11]] > cb)
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
								else if( p[offsets[5]] < c_b)
									if( p[offsets[10]] > cb)
										if( p[offsets[11]] > cb)
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else if( p[offsets[10]] < c_b)
										if( p[offsets[6]] < c_b)
											if( p[offsets[7]] < c_b)
												if( p[offsets[8]] < c_b)
													if( p[offsets[9]] < c_b)
														if( p[offsets[11]] < c_b)
															if( p[offsets[12]] < c_b)
																if( p[offsets[13]] < c_b)
																	if( p[offsets[14]] < c_b)
																		if( p[offsets[15]] < c_b)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									if( p[offsets[10]] > cb)
										if( p[offsets[11]] > cb)
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
							else if( p[offsets[4]] < c_b)
								if( p[offsets[15]] > cb)
									if( p[offsets[9]] > cb)
										if( p[offsets[10]] > cb)
											if( p[offsets[11]] > cb)
												if( p[offsets[12]] > cb)
													if( p[offsets[13]] > cb)
														if( p[offsets[14]] > cb)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else if( p[offsets[9]] < c_b)
										if( p[offsets[5]] < c_b)
											if( p[offsets[6]] < c_b)
												if( p[offsets[7]] < c_b)
													if( p[offsets[8]] < c_b)
														if( p[offsets[10]] < c_b)
															if( p[offsets[11]] < c_b)
																if( p[offsets[12]] < c_b)
																	if( p[offsets[13]] < c_b)
																		if( p[offsets[14]] < c_b)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									if( p[offsets[5]] < c_b)
										if( p[offsets[6]] < c_b)
											if( p[offsets[7]] < c_b)
												if( p[offsets[8]] < c_b)
													if( p[offsets[9]] < c_b)
														if( p[offsets[10]] < c_b)
															if( p[offsets[11]] < c_b)
																if( p[offsets[12]] < c_b)
																	if( p[offsets[13]] < c_b)
																		if( p[offsets[14]] < c_b)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
							else
								if( p[offsets[9]] > cb)
									if( p[offsets[10]] > cb)
										if( p[offsets[11]] > cb)
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else if( p[offsets[9]] < c_b)
									if( p[offsets[5]] < c_b)
										if( p[offsets[6]] < c_b)
											if( p[offsets[7]] < c_b)
												if( p[offsets[8]] < c_b)
													if( p[offsets[10]] < c_b)
														if( p[offsets[11]] < c_b)
															if( p[offsets[12]] < c_b)
																if( p[offsets[13]] < c_b)
																	if( p[offsets[14]] < c_b)
																		if( p[offsets[15]] < c_b)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
						else if( p[offsets[3]] < c_b)
							if( p[offsets[14]] > cb)
								if( p[offsets[8]] > cb)
									if( p[offsets[9]] > cb)
										if( p[offsets[10]] > cb)
											if( p[offsets[11]] > cb)
												if( p[offsets[12]] > cb)
													if( p[offsets[13]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	if( p[offsets[6]] > cb)
																		if( p[offsets[7]] > cb)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else if( p[offsets[8]] < c_b)
									if( p[offsets[4]] < c_b)
										if( p[offsets[5]] < c_b)
											if( p[offsets[6]] < c_b)
												if( p[offsets[7]] < c_b)
													if( p[offsets[9]] < c_b)
														if( p[offsets[10]] < c_b)
															if( p[offsets[11]] < c_b)
																if( p[offsets[12]] < c_b)
																	if( p[offsets[13]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else if( p[offsets[14]] < c_b)
								if( p[offsets[5]] < c_b)
									if( p[offsets[6]] < c_b)
										if( p[offsets[7]] < c_b)
											if( p[offsets[8]] < c_b)
												if( p[offsets[9]] < c_b)
													if( p[offsets[10]] < c_b)
														if( p[offsets[11]] < c_b)
															if( p[offsets[12]] < c_b)
																if( p[offsets[13]] < c_b)
																	if( p[offsets[4]] < c_b)
																		goto is_a_corner;
																	else
																		if( p[offsets[15]] < c_b)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								if( p[offsets[4]] < c_b)
									if( p[offsets[5]] < c_b)
										if( p[offsets[6]] < c_b)
											if( p[offsets[7]] < c_b)
												if( p[offsets[8]] < c_b)
													if( p[offsets[9]] < c_b)
														if( p[offsets[10]] < c_b)
															if( p[offsets[11]] < c_b)
																if( p[offsets[12]] < c_b)
																	if( p[offsets[13]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
						else
							if( p[offsets[8]] > cb)
								if( p[offsets[9]] > cb)
									if( p[offsets[10]] > cb)
										if( p[offsets[11]] > cb)
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	if( p[offsets[6]] > cb)
																		if( p[offsets[7]] > cb)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else if( p[offsets[8]] < c_b)
								if( p[offsets[5]] < c_b)
									if( p[offsets[6]] < c_b)
										if( p[offsets[7]] < c_b)
											if( p[offsets[9]] < c_b)
												if( p[offsets[10]] < c_b)
													if( p[offsets[11]] < c_b)
														if( p[offsets[12]] < c_b)
															if( p[offsets[13]] < c_b)
																if( p[offsets[14]] < c_b)
																	if( p[offsets[4]] < c_b)
																		goto is_a_corner;
																	else
																		if( p[offsets[15]] < c_b)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
					else if( p[offsets[2]] < c_b)
						if( p[offsets[7]] > cb)
							if( p[offsets[8]] > cb)
								if( p[offsets[9]] > cb)
									if( p[offsets[10]] > cb)
										if( p[offsets[11]] > cb)
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	if( p[offsets[6]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[3]] > cb)
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	if( p[offsets[6]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else if( p[offsets[7]] < c_b)
							if( p[offsets[5]] < c_b)
								if( p[offsets[6]] < c_b)
									if( p[offsets[8]] < c_b)
										if( p[offsets[9]] < c_b)
											if( p[offsets[10]] < c_b)
												if( p[offsets[11]] < c_b)
													if( p[offsets[12]] < c_b)
														if( p[offsets[4]] < c_b)
															if( p[offsets[3]] < c_b)
																goto is_a_corner;
															else
																if( p[offsets[13]] < c_b)
																	if( p[offsets[14]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
														else
															if( p[offsets[13]] < c_b)
																if( p[offsets[14]] < c_b)
																	if( p[offsets[15]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else
						if( p[offsets[7]] > cb)
							if( p[offsets[8]] > cb)
								if( p[offsets[9]] > cb)
									if( p[offsets[10]] > cb)
										if( p[offsets[11]] > cb)
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	if( p[offsets[6]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[3]] > cb)
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	if( p[offsets[6]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else if( p[offsets[7]] < c_b)
							if( p[offsets[5]] < c_b)
								if( p[offsets[6]] < c_b)
									if( p[offsets[8]] < c_b)
										if( p[offsets[9]] < c_b)
											if( p[offsets[10]] < c_b)
												if( p[offsets[11]] < c_b)
													if( p[offsets[12]] < c_b)
														if( p[offsets[13]] < c_b)
															if( p[offsets[4]] < c_b)
																if( p[offsets[3]] < c_b)
																	goto is_a_corner;
																else
																	if( p[offsets[14]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
															else
																if( p[offsets[14]] < c_b)
																	if( p[offsets[15]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
				else if( p[offsets[1]] < c_b)
					if( p[offsets[6]] > cb)
						if( p[offsets[7]] > cb)
							if( p[offsets[8]] > cb)
								if( p[offsets[9]] > cb)
									if( p[offsets[10]] > cb)
										if( p[offsets[11]] > cb)
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[3]] > cb)
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[2]] > cb)
														if( p[offsets[3]] > cb)
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else if( p[offsets[6]] < c_b)
						if( p[offsets[5]] < c_b)
							if( p[offsets[7]] < c_b)
								if( p[offsets[8]] < c_b)
									if( p[offsets[9]] < c_b)
										if( p[offsets[10]] < c_b)
											if( p[offsets[11]] < c_b)
												if( p[offsets[4]] < c_b)
													if( p[offsets[3]] < c_b)
														if( p[offsets[2]] < c_b)
															goto is_a_corner;
														else
															if( p[offsets[12]] < c_b)
																if( p[offsets[13]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[12]] < c_b)
															if( p[offsets[13]] < c_b)
																if( p[offsets[14]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[12]] < c_b)
														if( p[offsets[13]] < c_b)
															if( p[offsets[14]] < c_b)
																if( p[offsets[15]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else
						goto is_not_a_corner;
				else
					if( p[offsets[6]] > cb)
						if( p[offsets[7]] > cb)
							if( p[offsets[8]] > cb)
								if( p[offsets[9]] > cb)
									if( p[offsets[10]] > cb)
										if( p[offsets[11]] > cb)
											if( p[offsets[12]] > cb)
												if( p[offsets[13]] > cb)
													if( p[offsets[14]] > cb)
														if( p[offsets[15]] > cb)
															goto is_a_corner;
														else
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[3]] > cb)
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[2]] > cb)
														if( p[offsets[3]] > cb)
															if( p[offsets[4]] > cb)
																if( p[offsets[5]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else if( p[offsets[6]] < c_b)
						if( p[offsets[5]] < c_b)
							if( p[offsets[7]] < c_b)
								if( p[offsets[8]] < c_b)
									if( p[offsets[9]] < c_b)
										if( p[offsets[10]] < c_b)
											if( p[offsets[11]] < c_b)
												if( p[offsets[12]] < c_b)
													if( p[offsets[4]] < c_b)
														if( p[offsets[3]] < c_b)
															if( p[offsets[2]] < c_b)
																goto is_a_corner;
															else
																if( p[offsets[13]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
														else
															if( p[offsets[13]] < c_b)
																if( p[offsets[14]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[13]] < c_b)
															if( p[offsets[14]] < c_b)
																if( p[offsets[15]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else
						goto is_not_a_corner;
			else if( p[offsets[0]] < c_b)
				if( p[offsets[1]] > cb)
					if( p[offsets[6]] > cb)
						if( p[offsets[5]] > cb)
							if( p[offsets[7]] > cb)
								if( p[offsets[8]] > cb)
									if( p[offsets[9]] > cb)
										if( p[offsets[10]] > cb)
											if( p[offsets[11]] > cb)
												if( p[offsets[4]] > cb)
													if( p[offsets[3]] > cb)
														if( p[offsets[2]] > cb)
															goto is_a_corner;
														else
															if( p[offsets[12]] > cb)
																if( p[offsets[13]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[12]] > cb)
															if( p[offsets[13]] > cb)
																if( p[offsets[14]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[12]] > cb)
														if( p[offsets[13]] > cb)
															if( p[offsets[14]] > cb)
																if( p[offsets[15]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else if( p[offsets[6]] < c_b)
						if( p[offsets[7]] < c_b)
							if( p[offsets[8]] < c_b)
								if( p[offsets[9]] < c_b)
									if( p[offsets[10]] < c_b)
										if( p[offsets[11]] < c_b)
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[3]] < c_b)
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[2]] < c_b)
														if( p[offsets[3]] < c_b)
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else
						goto is_not_a_corner;
				else if( p[offsets[1]] < c_b)
					if( p[offsets[2]] > cb)
						if( p[offsets[7]] > cb)
							if( p[offsets[5]] > cb)
								if( p[offsets[6]] > cb)
									if( p[offsets[8]] > cb)
										if( p[offsets[9]] > cb)
											if( p[offsets[10]] > cb)
												if( p[offsets[11]] > cb)
													if( p[offsets[12]] > cb)
														if( p[offsets[4]] > cb)
															if( p[offsets[3]] > cb)
																goto is_a_corner;
															else
																if( p[offsets[13]] > cb)
																	if( p[offsets[14]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
														else
															if( p[offsets[13]] > cb)
																if( p[offsets[14]] > cb)
																	if( p[offsets[15]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else if( p[offsets[7]] < c_b)
							if( p[offsets[8]] < c_b)
								if( p[offsets[9]] < c_b)
									if( p[offsets[10]] < c_b)
										if( p[offsets[11]] < c_b)
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	if( p[offsets[6]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[3]] < c_b)
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	if( p[offsets[6]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else if( p[offsets[2]] < c_b)
						if( p[offsets[3]] > cb)
							if( p[offsets[14]] > cb)
								if( p[offsets[5]] > cb)
									if( p[offsets[6]] > cb)
										if( p[offsets[7]] > cb)
											if( p[offsets[8]] > cb)
												if( p[offsets[9]] > cb)
													if( p[offsets[10]] > cb)
														if( p[offsets[11]] > cb)
															if( p[offsets[12]] > cb)
																if( p[offsets[13]] > cb)
																	if( p[offsets[4]] > cb)
																		goto is_a_corner;
																	else
																		if( p[offsets[15]] > cb)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else if( p[offsets[14]] < c_b)
								if( p[offsets[8]] > cb)
									if( p[offsets[4]] > cb)
										if( p[offsets[5]] > cb)
											if( p[offsets[6]] > cb)
												if( p[offsets[7]] > cb)
													if( p[offsets[9]] > cb)
														if( p[offsets[10]] > cb)
															if( p[offsets[11]] > cb)
																if( p[offsets[12]] > cb)
																	if( p[offsets[13]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else if( p[offsets[8]] < c_b)
									if( p[offsets[9]] < c_b)
										if( p[offsets[10]] < c_b)
											if( p[offsets[11]] < c_b)
												if( p[offsets[12]] < c_b)
													if( p[offsets[13]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	if( p[offsets[6]] < c_b)
																		if( p[offsets[7]] < c_b)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								if( p[offsets[4]] > cb)
									if( p[offsets[5]] > cb)
										if( p[offsets[6]] > cb)
											if( p[offsets[7]] > cb)
												if( p[offsets[8]] > cb)
													if( p[offsets[9]] > cb)
														if( p[offsets[10]] > cb)
															if( p[offsets[11]] > cb)
																if( p[offsets[12]] > cb)
																	if( p[offsets[13]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
						else if( p[offsets[3]] < c_b)
							if( p[offsets[4]] > cb)
								if( p[offsets[15]] < c_b)
									if( p[offsets[9]] > cb)
										if( p[offsets[5]] > cb)
											if( p[offsets[6]] > cb)
												if( p[offsets[7]] > cb)
													if( p[offsets[8]] > cb)
														if( p[offsets[10]] > cb)
															if( p[offsets[11]] > cb)
																if( p[offsets[12]] > cb)
																	if( p[offsets[13]] > cb)
																		if( p[offsets[14]] > cb)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else if( p[offsets[9]] < c_b)
										if( p[offsets[10]] < c_b)
											if( p[offsets[11]] < c_b)
												if( p[offsets[12]] < c_b)
													if( p[offsets[13]] < c_b)
														if( p[offsets[14]] < c_b)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									if( p[offsets[5]] > cb)
										if( p[offsets[6]] > cb)
											if( p[offsets[7]] > cb)
												if( p[offsets[8]] > cb)
													if( p[offsets[9]] > cb)
														if( p[offsets[10]] > cb)
															if( p[offsets[11]] > cb)
																if( p[offsets[12]] > cb)
																	if( p[offsets[13]] > cb)
																		if( p[offsets[14]] > cb)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
							else if( p[offsets[4]] < c_b)
								if( p[offsets[5]] > cb)
									if( p[offsets[10]] > cb)
										if( p[offsets[6]] > cb)
											if( p[offsets[7]] > cb)
												if( p[offsets[8]] > cb)
													if( p[offsets[9]] > cb)
														if( p[offsets[11]] > cb)
															if( p[offsets[12]] > cb)
																if( p[offsets[13]] > cb)
																	if( p[offsets[14]] > cb)
																		if( p[offsets[15]] > cb)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else if( p[offsets[10]] < c_b)
										if( p[offsets[11]] < c_b)
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else if( p[offsets[5]] < c_b)
									if( p[offsets[6]] < c_b)
										if( p[offsets[7]] < c_b)
											if( p[offsets[8]] < c_b)
												if( p[offsets[9]] < c_b)
													if( p[offsets[10]] < c_b)
														goto is_a_corner;
													else
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
										else
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
									else
										if( p[offsets[11]] < c_b)
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
								else
									if( p[offsets[10]] < c_b)
										if( p[offsets[11]] < c_b)
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
							else
								if( p[offsets[9]] > cb)
									if( p[offsets[5]] > cb)
										if( p[offsets[6]] > cb)
											if( p[offsets[7]] > cb)
												if( p[offsets[8]] > cb)
													if( p[offsets[10]] > cb)
														if( p[offsets[11]] > cb)
															if( p[offsets[12]] > cb)
																if( p[offsets[13]] > cb)
																	if( p[offsets[14]] > cb)
																		if( p[offsets[15]] > cb)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else if( p[offsets[9]] < c_b)
									if( p[offsets[10]] < c_b)
										if( p[offsets[11]] < c_b)
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
						else
							if( p[offsets[8]] > cb)
								if( p[offsets[5]] > cb)
									if( p[offsets[6]] > cb)
										if( p[offsets[7]] > cb)
											if( p[offsets[9]] > cb)
												if( p[offsets[10]] > cb)
													if( p[offsets[11]] > cb)
														if( p[offsets[12]] > cb)
															if( p[offsets[13]] > cb)
																if( p[offsets[14]] > cb)
																	if( p[offsets[4]] > cb)
																		goto is_a_corner;
																	else
																		if( p[offsets[15]] > cb)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else if( p[offsets[8]] < c_b)
								if( p[offsets[9]] < c_b)
									if( p[offsets[10]] < c_b)
										if( p[offsets[11]] < c_b)
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	if( p[offsets[6]] < c_b)
																		if( p[offsets[7]] < c_b)
																			goto is_a_corner;
																		else
																			goto is_not_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
					else
						if( p[offsets[7]] > cb)
							if( p[offsets[5]] > cb)
								if( p[offsets[6]] > cb)
									if( p[offsets[8]] > cb)
										if( p[offsets[9]] > cb)
											if( p[offsets[10]] > cb)
												if( p[offsets[11]] > cb)
													if( p[offsets[12]] > cb)
														if( p[offsets[13]] > cb)
															if( p[offsets[4]] > cb)
																if( p[offsets[3]] > cb)
																	goto is_a_corner;
																else
																	if( p[offsets[14]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
															else
																if( p[offsets[14]] > cb)
																	if( p[offsets[15]] > cb)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else if( p[offsets[7]] < c_b)
							if( p[offsets[8]] < c_b)
								if( p[offsets[9]] < c_b)
									if( p[offsets[10]] < c_b)
										if( p[offsets[11]] < c_b)
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	if( p[offsets[6]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[3]] < c_b)
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	if( p[offsets[6]] < c_b)
																		goto is_a_corner;
																	else
																		goto is_not_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
				else
					if( p[offsets[6]] > cb)
						if( p[offsets[5]] > cb)
							if( p[offsets[7]] > cb)
								if( p[offsets[8]] > cb)
									if( p[offsets[9]] > cb)
										if( p[offsets[10]] > cb)
											if( p[offsets[11]] > cb)
												if( p[offsets[12]] > cb)
													if( p[offsets[4]] > cb)
														if( p[offsets[3]] > cb)
															if( p[offsets[2]] > cb)
																goto is_a_corner;
															else
																if( p[offsets[13]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
														else
															if( p[offsets[13]] > cb)
																if( p[offsets[14]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[13]] > cb)
															if( p[offsets[14]] > cb)
																if( p[offsets[15]] > cb)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else if( p[offsets[6]] < c_b)
						if( p[offsets[7]] < c_b)
							if( p[offsets[8]] < c_b)
								if( p[offsets[9]] < c_b)
									if( p[offsets[10]] < c_b)
										if( p[offsets[11]] < c_b)
											if( p[offsets[12]] < c_b)
												if( p[offsets[13]] < c_b)
													if( p[offsets[14]] < c_b)
														if( p[offsets[15]] < c_b)
															goto is_a_corner;
														else
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[3]] < c_b)
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[2]] < c_b)
														if( p[offsets[3]] < c_b)
															if( p[offsets[4]] < c_b)
																if( p[offsets[5]] < c_b)
																	goto is_a_corner;
																else
																	goto is_not_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else
						goto is_not_a_corner;
			else
				if( p[offsets[5]] > cb)
					if( p[offsets[6]] > cb)
						if( p[offsets[7]] > cb)
							if( p[offsets[8]] > cb)
								if( p[offsets[9]] > cb)
									if( p[offsets[10]] > cb)
										if( p[offsets[11]] > cb)
											if( p[offsets[4]] > cb)
												if( p[offsets[3]] > cb)
													if( p[offsets[2]] > cb)
														if( p[offsets[1]] > cb)
															goto is_a_corner;
														else
															if( p[offsets[12]] > cb)
																goto is_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[12]] > cb)
															if( p[offsets[13]] > cb)
																goto is_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[12]] > cb)
														if( p[offsets[13]] > cb)
															if( p[offsets[14]] > cb)
																goto is_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												if( p[offsets[12]] > cb)
													if( p[offsets[13]] > cb)
														if( p[offsets[14]] > cb)
															if( p[offsets[15]] > cb)
																goto is_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else
						goto is_not_a_corner;
				else if( p[offsets[5]] < c_b)
					if( p[offsets[6]] < c_b)
						if( p[offsets[7]] < c_b)
							if( p[offsets[8]] < c_b)
								if( p[offsets[9]] < c_b)
									if( p[offsets[10]] < c_b)
										if( p[offsets[11]] < c_b)
											if( p[offsets[4]] < c_b)
												if( p[offsets[3]] < c_b)
													if( p[offsets[2]] < c_b)
														if( p[offsets[1]] < c_b)
															goto is_a_corner;
														else
															if( p[offsets[12]] < c_b)
																goto is_a_corner;
															else
																goto is_not_a_corner;
													else
														if( p[offsets[12]] < c_b)
															if( p[offsets[13]] < c_b)
																goto is_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
												else
													if( p[offsets[12]] < c_b)
														if( p[offsets[13]] < c_b)
															if( p[offsets[14]] < c_b)
																goto is_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
											else
												if( p[offsets[12]] < c_b)
													if( p[offsets[13]] < c_b)
														if( p[offsets[14]] < c_b)
															if( p[offsets[15]] < c_b)
																goto is_a_corner;
															else
																goto is_not_a_corner;
														else
															goto is_not_a_corner;
													else
														goto is_not_a_corner;
												else
													goto is_not_a_corner;
										else
											goto is_not_a_corner;
									else
										goto is_not_a_corner;
								else
									goto is_not_a_corner;
							else
								goto is_not_a_corner;
						else
							goto is_not_a_corner;
					else
						goto is_not_a_corner;
				else
					goto is_not_a_corner;

is_a_corner:
			bmin=b;
			goto end_if;

is_not_a_corner:
			bmax=b;
			goto end_if;

end_if:

			if(bmin == bmax - 1 || bmin == bmax)
				return bmin;
			b = (bmin + bmax) / 2;
		}
	}

	bool FAST::isCorner11( const uint8_t* p, const int* offsets, uint8_t threshold )
	{
		int cb = *p + threshold;
		int c_b= *p - threshold;
		if(p[offsets[0]] > cb)
			if(p[offsets[1]] > cb)
				if(p[offsets[2]] > cb)
					if(p[offsets[3]] > cb)
						if(p[offsets[4]] > cb)
							if(p[offsets[5]] > cb)
								if(p[offsets[6]] > cb)
									if(p[offsets[7]] > cb)
										if(p[offsets[8]] > cb)
											if(p[offsets[9]] > cb)
												if(p[offsets[10]] > cb)
												{}
												else
													if(p[offsets[15]] > cb)
													{}
													else
														return false;
											else
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														return false;
												else
													return false;
										else
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
									else
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
								else
									if(p[offsets[11]] > cb)
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
							else if(p[offsets[5]] < c_b)
								if(p[offsets[10]] > cb)
									if(p[offsets[11]] > cb)
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else if(p[offsets[10]] < c_b)
									if(p[offsets[6]] < c_b)
										if(p[offsets[7]] < c_b)
											if(p[offsets[8]] < c_b)
												if(p[offsets[9]] < c_b)
													if(p[offsets[11]] < c_b)
														if(p[offsets[12]] < c_b)
															if(p[offsets[13]] < c_b)
																if(p[offsets[14]] < c_b)
																	if(p[offsets[15]] < c_b)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								if(p[offsets[10]] > cb)
									if(p[offsets[11]] > cb)
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
						else if(p[offsets[4]] < c_b)
							if(p[offsets[15]] > cb)
								if(p[offsets[9]] > cb)
									if(p[offsets[10]] > cb)
										if(p[offsets[11]] > cb)
											if(p[offsets[12]] > cb)
												if(p[offsets[13]] > cb)
													if(p[offsets[14]] > cb)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else if(p[offsets[9]] < c_b)
									if(p[offsets[5]] < c_b)
										if(p[offsets[6]] < c_b)
											if(p[offsets[7]] < c_b)
												if(p[offsets[8]] < c_b)
													if(p[offsets[10]] < c_b)
														if(p[offsets[11]] < c_b)
															if(p[offsets[12]] < c_b)
																if(p[offsets[13]] < c_b)
																	if(p[offsets[14]] < c_b)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								if(p[offsets[5]] < c_b)
									if(p[offsets[6]] < c_b)
										if(p[offsets[7]] < c_b)
											if(p[offsets[8]] < c_b)
												if(p[offsets[9]] < c_b)
													if(p[offsets[10]] < c_b)
														if(p[offsets[11]] < c_b)
															if(p[offsets[12]] < c_b)
																if(p[offsets[13]] < c_b)
																	if(p[offsets[14]] < c_b)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
						else
							if(p[offsets[9]] > cb)
								if(p[offsets[10]] > cb)
									if(p[offsets[11]] > cb)
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else if(p[offsets[9]] < c_b)
								if(p[offsets[5]] < c_b)
									if(p[offsets[6]] < c_b)
										if(p[offsets[7]] < c_b)
											if(p[offsets[8]] < c_b)
												if(p[offsets[10]] < c_b)
													if(p[offsets[11]] < c_b)
														if(p[offsets[12]] < c_b)
															if(p[offsets[13]] < c_b)
																if(p[offsets[14]] < c_b)
																	if(p[offsets[15]] < c_b)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
					else if(p[offsets[3]] < c_b)
						if(p[offsets[14]] > cb)
							if(p[offsets[8]] > cb)
								if(p[offsets[9]] > cb)
									if(p[offsets[10]] > cb)
										if(p[offsets[11]] > cb)
											if(p[offsets[12]] > cb)
												if(p[offsets[13]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
																if(p[offsets[6]] > cb)
																	if(p[offsets[7]] > cb)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else if(p[offsets[8]] < c_b)
								if(p[offsets[4]] < c_b)
									if(p[offsets[5]] < c_b)
										if(p[offsets[6]] < c_b)
											if(p[offsets[7]] < c_b)
												if(p[offsets[9]] < c_b)
													if(p[offsets[10]] < c_b)
														if(p[offsets[11]] < c_b)
															if(p[offsets[12]] < c_b)
																if(p[offsets[13]] < c_b)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else if(p[offsets[14]] < c_b)
							if(p[offsets[5]] < c_b)
								if(p[offsets[6]] < c_b)
									if(p[offsets[7]] < c_b)
										if(p[offsets[8]] < c_b)
											if(p[offsets[9]] < c_b)
												if(p[offsets[10]] < c_b)
													if(p[offsets[11]] < c_b)
														if(p[offsets[12]] < c_b)
															if(p[offsets[13]] < c_b)
																if(p[offsets[4]] < c_b)
																{}
																else
																	if(p[offsets[15]] < c_b)
																	{}
																	else
																		return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							if(p[offsets[4]] < c_b)
								if(p[offsets[5]] < c_b)
									if(p[offsets[6]] < c_b)
										if(p[offsets[7]] < c_b)
											if(p[offsets[8]] < c_b)
												if(p[offsets[9]] < c_b)
													if(p[offsets[10]] < c_b)
														if(p[offsets[11]] < c_b)
															if(p[offsets[12]] < c_b)
																if(p[offsets[13]] < c_b)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
					else
						if(p[offsets[8]] > cb)
							if(p[offsets[9]] > cb)
								if(p[offsets[10]] > cb)
									if(p[offsets[11]] > cb)
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
																if(p[offsets[6]] > cb)
																	if(p[offsets[7]] > cb)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else if(p[offsets[8]] < c_b)
							if(p[offsets[5]] < c_b)
								if(p[offsets[6]] < c_b)
									if(p[offsets[7]] < c_b)
										if(p[offsets[9]] < c_b)
											if(p[offsets[10]] < c_b)
												if(p[offsets[11]] < c_b)
													if(p[offsets[12]] < c_b)
														if(p[offsets[13]] < c_b)
															if(p[offsets[14]] < c_b)
																if(p[offsets[4]] < c_b)
																{}
																else
																	if(p[offsets[15]] < c_b)
																	{}
																	else
																		return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
				else if(p[offsets[2]] < c_b)
					if(p[offsets[7]] > cb)
						if(p[offsets[8]] > cb)
							if(p[offsets[9]] > cb)
								if(p[offsets[10]] > cb)
									if(p[offsets[11]] > cb)
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
																if(p[offsets[6]] > cb)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													if(p[offsets[3]] > cb)
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
																if(p[offsets[6]] > cb)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else if(p[offsets[7]] < c_b)
						if(p[offsets[5]] < c_b)
							if(p[offsets[6]] < c_b)
								if(p[offsets[8]] < c_b)
									if(p[offsets[9]] < c_b)
										if(p[offsets[10]] < c_b)
											if(p[offsets[11]] < c_b)
												if(p[offsets[12]] < c_b)
													if(p[offsets[4]] < c_b)
														if(p[offsets[3]] < c_b)
														{}
														else
															if(p[offsets[13]] < c_b)
																if(p[offsets[14]] < c_b)
																{}
																else
																	return false;
															else
																return false;
													else
														if(p[offsets[13]] < c_b)
															if(p[offsets[14]] < c_b)
																if(p[offsets[15]] < c_b)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else
					if(p[offsets[7]] > cb)
						if(p[offsets[8]] > cb)
							if(p[offsets[9]] > cb)
								if(p[offsets[10]] > cb)
									if(p[offsets[11]] > cb)
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
																if(p[offsets[6]] > cb)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													if(p[offsets[3]] > cb)
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
																if(p[offsets[6]] > cb)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else if(p[offsets[7]] < c_b)
						if(p[offsets[5]] < c_b)
							if(p[offsets[6]] < c_b)
								if(p[offsets[8]] < c_b)
									if(p[offsets[9]] < c_b)
										if(p[offsets[10]] < c_b)
											if(p[offsets[11]] < c_b)
												if(p[offsets[12]] < c_b)
													if(p[offsets[13]] < c_b)
														if(p[offsets[4]] < c_b)
															if(p[offsets[3]] < c_b)
															{}
															else
																if(p[offsets[14]] < c_b)
																{}
																else
																	return false;
														else
															if(p[offsets[14]] < c_b)
																if(p[offsets[15]] < c_b)
																{}
																else
																	return false;
															else
																return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
			else if(p[offsets[1]] < c_b)
				if(p[offsets[6]] > cb)
					if(p[offsets[7]] > cb)
						if(p[offsets[8]] > cb)
							if(p[offsets[9]] > cb)
								if(p[offsets[10]] > cb)
									if(p[offsets[11]] > cb)
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
															{}
															else
																return false;
														else
															return false;
												else
													if(p[offsets[3]] > cb)
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
											else
												if(p[offsets[2]] > cb)
													if(p[offsets[3]] > cb)
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else if(p[offsets[6]] < c_b)
					if(p[offsets[5]] < c_b)
						if(p[offsets[7]] < c_b)
							if(p[offsets[8]] < c_b)
								if(p[offsets[9]] < c_b)
									if(p[offsets[10]] < c_b)
										if(p[offsets[11]] < c_b)
											if(p[offsets[4]] < c_b)
												if(p[offsets[3]] < c_b)
													if(p[offsets[2]] < c_b)
													{}
													else
														if(p[offsets[12]] < c_b)
															if(p[offsets[13]] < c_b)
															{}
															else
																return false;
														else
															return false;
												else
													if(p[offsets[12]] < c_b)
														if(p[offsets[13]] < c_b)
															if(p[offsets[14]] < c_b)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
											else
												if(p[offsets[12]] < c_b)
													if(p[offsets[13]] < c_b)
														if(p[offsets[14]] < c_b)
															if(p[offsets[15]] < c_b)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else
					return false;
			else
				if(p[offsets[6]] > cb)
					if(p[offsets[7]] > cb)
						if(p[offsets[8]] > cb)
							if(p[offsets[9]] > cb)
								if(p[offsets[10]] > cb)
									if(p[offsets[11]] > cb)
										if(p[offsets[12]] > cb)
											if(p[offsets[13]] > cb)
												if(p[offsets[14]] > cb)
													if(p[offsets[15]] > cb)
													{}
													else
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
															{}
															else
																return false;
														else
															return false;
												else
													if(p[offsets[3]] > cb)
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
											else
												if(p[offsets[2]] > cb)
													if(p[offsets[3]] > cb)
														if(p[offsets[4]] > cb)
															if(p[offsets[5]] > cb)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else if(p[offsets[6]] < c_b)
					if(p[offsets[5]] < c_b)
						if(p[offsets[7]] < c_b)
							if(p[offsets[8]] < c_b)
								if(p[offsets[9]] < c_b)
									if(p[offsets[10]] < c_b)
										if(p[offsets[11]] < c_b)
											if(p[offsets[12]] < c_b)
												if(p[offsets[4]] < c_b)
													if(p[offsets[3]] < c_b)
														if(p[offsets[2]] < c_b)
														{}
														else
															if(p[offsets[13]] < c_b)
															{}
															else
																return false;
													else
														if(p[offsets[13]] < c_b)
															if(p[offsets[14]] < c_b)
															{}
															else
																return false;
														else
															return false;
												else
													if(p[offsets[13]] < c_b)
														if(p[offsets[14]] < c_b)
															if(p[offsets[15]] < c_b)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else
					return false;
		else if(p[offsets[0]] < c_b)
			if(p[offsets[1]] > cb)
				if(p[offsets[6]] > cb)
					if(p[offsets[5]] > cb)
						if(p[offsets[7]] > cb)
							if(p[offsets[8]] > cb)
								if(p[offsets[9]] > cb)
									if(p[offsets[10]] > cb)
										if(p[offsets[11]] > cb)
											if(p[offsets[4]] > cb)
												if(p[offsets[3]] > cb)
													if(p[offsets[2]] > cb)
													{}
													else
														if(p[offsets[12]] > cb)
															if(p[offsets[13]] > cb)
															{}
															else
																return false;
														else
															return false;
												else
													if(p[offsets[12]] > cb)
														if(p[offsets[13]] > cb)
															if(p[offsets[14]] > cb)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
											else
												if(p[offsets[12]] > cb)
													if(p[offsets[13]] > cb)
														if(p[offsets[14]] > cb)
															if(p[offsets[15]] > cb)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else if(p[offsets[6]] < c_b)
					if(p[offsets[7]] < c_b)
						if(p[offsets[8]] < c_b)
							if(p[offsets[9]] < c_b)
								if(p[offsets[10]] < c_b)
									if(p[offsets[11]] < c_b)
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
															{}
															else
																return false;
														else
															return false;
												else
													if(p[offsets[3]] < c_b)
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
											else
												if(p[offsets[2]] < c_b)
													if(p[offsets[3]] < c_b)
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else
					return false;
			else if(p[offsets[1]] < c_b)
				if(p[offsets[2]] > cb)
					if(p[offsets[7]] > cb)
						if(p[offsets[5]] > cb)
							if(p[offsets[6]] > cb)
								if(p[offsets[8]] > cb)
									if(p[offsets[9]] > cb)
										if(p[offsets[10]] > cb)
											if(p[offsets[11]] > cb)
												if(p[offsets[12]] > cb)
													if(p[offsets[4]] > cb)
														if(p[offsets[3]] > cb)
														{}
														else
															if(p[offsets[13]] > cb)
																if(p[offsets[14]] > cb)
																{}
																else
																	return false;
															else
																return false;
													else
														if(p[offsets[13]] > cb)
															if(p[offsets[14]] > cb)
																if(p[offsets[15]] > cb)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else if(p[offsets[7]] < c_b)
						if(p[offsets[8]] < c_b)
							if(p[offsets[9]] < c_b)
								if(p[offsets[10]] < c_b)
									if(p[offsets[11]] < c_b)
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
																if(p[offsets[6]] < c_b)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													if(p[offsets[3]] < c_b)
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
																if(p[offsets[6]] < c_b)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else if(p[offsets[2]] < c_b)
					if(p[offsets[3]] > cb)
						if(p[offsets[14]] > cb)
							if(p[offsets[5]] > cb)
								if(p[offsets[6]] > cb)
									if(p[offsets[7]] > cb)
										if(p[offsets[8]] > cb)
											if(p[offsets[9]] > cb)
												if(p[offsets[10]] > cb)
													if(p[offsets[11]] > cb)
														if(p[offsets[12]] > cb)
															if(p[offsets[13]] > cb)
																if(p[offsets[4]] > cb)
																{}
																else
																	if(p[offsets[15]] > cb)
																	{}
																	else
																		return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else if(p[offsets[14]] < c_b)
							if(p[offsets[8]] > cb)
								if(p[offsets[4]] > cb)
									if(p[offsets[5]] > cb)
										if(p[offsets[6]] > cb)
											if(p[offsets[7]] > cb)
												if(p[offsets[9]] > cb)
													if(p[offsets[10]] > cb)
														if(p[offsets[11]] > cb)
															if(p[offsets[12]] > cb)
																if(p[offsets[13]] > cb)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else if(p[offsets[8]] < c_b)
								if(p[offsets[9]] < c_b)
									if(p[offsets[10]] < c_b)
										if(p[offsets[11]] < c_b)
											if(p[offsets[12]] < c_b)
												if(p[offsets[13]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
																if(p[offsets[6]] < c_b)
																	if(p[offsets[7]] < c_b)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							if(p[offsets[4]] > cb)
								if(p[offsets[5]] > cb)
									if(p[offsets[6]] > cb)
										if(p[offsets[7]] > cb)
											if(p[offsets[8]] > cb)
												if(p[offsets[9]] > cb)
													if(p[offsets[10]] > cb)
														if(p[offsets[11]] > cb)
															if(p[offsets[12]] > cb)
																if(p[offsets[13]] > cb)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
					else if(p[offsets[3]] < c_b)
						if(p[offsets[4]] > cb)
							if(p[offsets[15]] < c_b)
								if(p[offsets[9]] > cb)
									if(p[offsets[5]] > cb)
										if(p[offsets[6]] > cb)
											if(p[offsets[7]] > cb)
												if(p[offsets[8]] > cb)
													if(p[offsets[10]] > cb)
														if(p[offsets[11]] > cb)
															if(p[offsets[12]] > cb)
																if(p[offsets[13]] > cb)
																	if(p[offsets[14]] > cb)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else if(p[offsets[9]] < c_b)
									if(p[offsets[10]] < c_b)
										if(p[offsets[11]] < c_b)
											if(p[offsets[12]] < c_b)
												if(p[offsets[13]] < c_b)
													if(p[offsets[14]] < c_b)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								if(p[offsets[5]] > cb)
									if(p[offsets[6]] > cb)
										if(p[offsets[7]] > cb)
											if(p[offsets[8]] > cb)
												if(p[offsets[9]] > cb)
													if(p[offsets[10]] > cb)
														if(p[offsets[11]] > cb)
															if(p[offsets[12]] > cb)
																if(p[offsets[13]] > cb)
																	if(p[offsets[14]] > cb)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
						else if(p[offsets[4]] < c_b)
							if(p[offsets[5]] > cb)
								if(p[offsets[10]] > cb)
									if(p[offsets[6]] > cb)
										if(p[offsets[7]] > cb)
											if(p[offsets[8]] > cb)
												if(p[offsets[9]] > cb)
													if(p[offsets[11]] > cb)
														if(p[offsets[12]] > cb)
															if(p[offsets[13]] > cb)
																if(p[offsets[14]] > cb)
																	if(p[offsets[15]] > cb)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else if(p[offsets[10]] < c_b)
									if(p[offsets[11]] < c_b)
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else if(p[offsets[5]] < c_b)
								if(p[offsets[6]] < c_b)
									if(p[offsets[7]] < c_b)
										if(p[offsets[8]] < c_b)
											if(p[offsets[9]] < c_b)
												if(p[offsets[10]] < c_b)
												{}
												else
													if(p[offsets[15]] < c_b)
													{}
													else
														return false;
											else
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														return false;
												else
													return false;
										else
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
									else
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
								else
									if(p[offsets[11]] < c_b)
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
							else
								if(p[offsets[10]] < c_b)
									if(p[offsets[11]] < c_b)
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
						else
							if(p[offsets[9]] > cb)
								if(p[offsets[5]] > cb)
									if(p[offsets[6]] > cb)
										if(p[offsets[7]] > cb)
											if(p[offsets[8]] > cb)
												if(p[offsets[10]] > cb)
													if(p[offsets[11]] > cb)
														if(p[offsets[12]] > cb)
															if(p[offsets[13]] > cb)
																if(p[offsets[14]] > cb)
																	if(p[offsets[15]] > cb)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else if(p[offsets[9]] < c_b)
								if(p[offsets[10]] < c_b)
									if(p[offsets[11]] < c_b)
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
					else
						if(p[offsets[8]] > cb)
							if(p[offsets[5]] > cb)
								if(p[offsets[6]] > cb)
									if(p[offsets[7]] > cb)
										if(p[offsets[9]] > cb)
											if(p[offsets[10]] > cb)
												if(p[offsets[11]] > cb)
													if(p[offsets[12]] > cb)
														if(p[offsets[13]] > cb)
															if(p[offsets[14]] > cb)
																if(p[offsets[4]] > cb)
																{}
																else
																	if(p[offsets[15]] > cb)
																	{}
																	else
																		return false;
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else if(p[offsets[8]] < c_b)
							if(p[offsets[9]] < c_b)
								if(p[offsets[10]] < c_b)
									if(p[offsets[11]] < c_b)
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
																if(p[offsets[6]] < c_b)
																	if(p[offsets[7]] < c_b)
																	{}
																	else
																		return false;
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
				else
					if(p[offsets[7]] > cb)
						if(p[offsets[5]] > cb)
							if(p[offsets[6]] > cb)
								if(p[offsets[8]] > cb)
									if(p[offsets[9]] > cb)
										if(p[offsets[10]] > cb)
											if(p[offsets[11]] > cb)
												if(p[offsets[12]] > cb)
													if(p[offsets[13]] > cb)
														if(p[offsets[4]] > cb)
															if(p[offsets[3]] > cb)
															{}
															else
																if(p[offsets[14]] > cb)
																{}
																else
																	return false;
														else
															if(p[offsets[14]] > cb)
																if(p[offsets[15]] > cb)
																{}
																else
																	return false;
															else
																return false;
													else
														return false;
												else
													return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else if(p[offsets[7]] < c_b)
						if(p[offsets[8]] < c_b)
							if(p[offsets[9]] < c_b)
								if(p[offsets[10]] < c_b)
									if(p[offsets[11]] < c_b)
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
																if(p[offsets[6]] < c_b)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
												else
													if(p[offsets[3]] < c_b)
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
																if(p[offsets[6]] < c_b)
																{}
																else
																	return false;
															else
																return false;
														else
															return false;
													else
														return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
			else
				if(p[offsets[6]] > cb)
					if(p[offsets[5]] > cb)
						if(p[offsets[7]] > cb)
							if(p[offsets[8]] > cb)
								if(p[offsets[9]] > cb)
									if(p[offsets[10]] > cb)
										if(p[offsets[11]] > cb)
											if(p[offsets[12]] > cb)
												if(p[offsets[4]] > cb)
													if(p[offsets[3]] > cb)
														if(p[offsets[2]] > cb)
														{}
														else
															if(p[offsets[13]] > cb)
															{}
															else
																return false;
													else
														if(p[offsets[13]] > cb)
															if(p[offsets[14]] > cb)
															{}
															else
																return false;
														else
															return false;
												else
													if(p[offsets[13]] > cb)
														if(p[offsets[14]] > cb)
															if(p[offsets[15]] > cb)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
											else
												return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else if(p[offsets[6]] < c_b)
					if(p[offsets[7]] < c_b)
						if(p[offsets[8]] < c_b)
							if(p[offsets[9]] < c_b)
								if(p[offsets[10]] < c_b)
									if(p[offsets[11]] < c_b)
										if(p[offsets[12]] < c_b)
											if(p[offsets[13]] < c_b)
												if(p[offsets[14]] < c_b)
													if(p[offsets[15]] < c_b)
													{}
													else
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
															{}
															else
																return false;
														else
															return false;
												else
													if(p[offsets[3]] < c_b)
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
											else
												if(p[offsets[2]] < c_b)
													if(p[offsets[3]] < c_b)
														if(p[offsets[4]] < c_b)
															if(p[offsets[5]] < c_b)
															{}
															else
																return false;
														else
															return false;
													else
														return false;
												else
													return false;
										else
											return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else
					return false;
		else
			if(p[offsets[5]] > cb)
				if(p[offsets[6]] > cb)
					if(p[offsets[7]] > cb)
						if(p[offsets[8]] > cb)
							if(p[offsets[9]] > cb)
								if(p[offsets[10]] > cb)
									if(p[offsets[11]] > cb)
										if(p[offsets[4]] > cb)
											if(p[offsets[3]] > cb)
												if(p[offsets[2]] > cb)
													if(p[offsets[1]] > cb)
													{}
													else
														if(p[offsets[12]] > cb)
														{}
														else
															return false;
												else
													if(p[offsets[12]] > cb)
														if(p[offsets[13]] > cb)
														{}
														else
															return false;
													else
														return false;
											else
												if(p[offsets[12]] > cb)
													if(p[offsets[13]] > cb)
														if(p[offsets[14]] > cb)
														{}
														else
															return false;
													else
														return false;
												else
													return false;
										else
											if(p[offsets[12]] > cb)
												if(p[offsets[13]] > cb)
													if(p[offsets[14]] > cb)
														if(p[offsets[15]] > cb)
														{}
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else
					return false;
			else if(p[offsets[5]] < c_b)
				if(p[offsets[6]] < c_b)
					if(p[offsets[7]] < c_b)
						if(p[offsets[8]] < c_b)
							if(p[offsets[9]] < c_b)
								if(p[offsets[10]] < c_b)
									if(p[offsets[11]] < c_b)
										if(p[offsets[4]] < c_b)
											if(p[offsets[3]] < c_b)
												if(p[offsets[2]] < c_b)
													if(p[offsets[1]] < c_b)
													{}
													else
														if(p[offsets[12]] < c_b)
														{}
														else
															return false;
												else
													if(p[offsets[12]] < c_b)
														if(p[offsets[13]] < c_b)
														{}
														else
															return false;
													else
														return false;
											else
												if(p[offsets[12]] < c_b)
													if(p[offsets[13]] < c_b)
														if(p[offsets[14]] < c_b)
														{}
														else
															return false;
													else
														return false;
												else
													return false;
										else
											if(p[offsets[12]] < c_b)
												if(p[offsets[13]] < c_b)
													if(p[offsets[14]] < c_b)
														if(p[offsets[15]] < c_b)
														{}
														else
															return false;
													else
														return false;
												else
													return false;
											else
												return false;
									else
										return false;
								else
									return false;
							else
								return false;
						else
							return false;
					else
						return false;
				else
					return false;
			else
				return false;
		return true;
	}
}
