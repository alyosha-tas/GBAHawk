using System;
using BizHawk.Common.NumberExtensions;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBHawkLink4x
{
	public partial class GBHawkLink4x : IEmulator, IVideoProvider, ISoundProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _controllerDeck.Definition;

		public bool FrameAdvance(IController controller, bool render, bool rendersound)
		{
			//Console.WriteLine("-----------------------FRAME-----------------------");

			if (_tracer.IsEnabled())
			{
				A.cpu.TraceCallback = s => _tracer.Put(s);
			}
			else
			{
				A.cpu.TraceCallback = null;
			}

			if (controller.IsPressed("P1 Power"))
			{
				A.HardReset();
			}
			if (controller.IsPressed("P2 Power"))
			{
				B.HardReset();
			}
			if (controller.IsPressed("P3 Power"))
			{
				C.HardReset();
			}
			if (controller.IsPressed("P4 Power") && !only_3_roms)
			{
				D.HardReset();
			}

			if (only_3_roms)
			{
				if (controller.IsPressed("Toggle Cable LR") |
				controller.IsPressed("Toggle Cable RC") |
				controller.IsPressed("Toggle Cable CL"))
				{
					// if any connection exists, disconnect it
					// otherwise connect in order of precedence
					// only one event can happen per frame, either a connection or disconnection
					if (_cableconnected_UD | _cableconnected_LR | _cableconnected_X)
					{
						_cableconnected_UD = _cableconnected_LR = _cableconnected_X = false;
						do_2_next_1 = false;
						do_2_next_2 = false;
					}
					else if (controller.IsPressed("Toggle Cable LR"))
					{
						_cableconnected_LR = true;
					}
					else if (controller.IsPressed("Toggle Cable RC"))
					{
						_cableconnected_X = true;
					}
					else if (controller.IsPressed("Toggle Cable CL"))
					{
						_cableconnected_UD = true;
					}

					Console.WriteLine("Cable connect status:");
					Console.WriteLine("LR: " + _cableconnected_UD);
					Console.WriteLine("RC: " + _cableconnected_LR);
					Console.WriteLine("CL: " + _cableconnected_X);
				}
			}
			else
			{
				if (controller.IsPressed("Toggle Cable UD") |
				controller.IsPressed("Toggle Cable LR") |
				controller.IsPressed("Toggle Cable X") |
				(controller.IsPressed("Toggle Cable 4x")))
				{
					// if any connection exists, disconnect it
					// otherwise connect in order of precedence
					// only one event can happen per frame, either a connection or disconnection
					if (_cableconnected_UD | _cableconnected_LR | _cableconnected_X | _cableconnected_4x)
					{
						_cableconnected_UD = _cableconnected_LR = _cableconnected_X = _cableconnected_4x = false;
						do_2_next_1 = false;
						do_2_next_2 = false;
					}
					else if (controller.IsPressed("Toggle Cable UD"))
					{
						_cableconnected_UD = true;
					}
					else if (controller.IsPressed("Toggle Cable LR"))
					{
						_cableconnected_LR = true;
					}
					else if (controller.IsPressed("Toggle Cable X"))
					{
						_cableconnected_X = true;
					}
					else if (controller.IsPressed("Toggle Cable 4x") && !only_3_roms)
					{
						_cableconnected_4x = true;
						is_pinging = false;
						is_transmitting = false;
					}

					Console.WriteLine("Cable connect status:");
					Console.WriteLine("UD: " + _cableconnected_UD);
					Console.WriteLine("LR: " + _cableconnected_LR);
					Console.WriteLine("X: " + _cableconnected_X);
					Console.WriteLine("4x: " + _cableconnected_4x);
				}
			}

			_islag = true;

			GetControllerState(controller);

			do_frame_fill = false;

			if (_cableconnected_4x)
			{
				do_frame_4x();
			}
			else
			{
				if (only_3_roms)
				{
					do_frame_3();
				}
				else
				{
					do_frame_2x2();
				}				
			}

			if (A.vblank_rise)
			{
				do_frame_fill = true;
				A.vblank_rise = false;
			}

			if (B.vblank_rise)
			{
				do_frame_fill = true;
				B.vblank_rise = false;
			}

			if (C.vblank_rise)
			{
				do_frame_fill = true;
				C.vblank_rise = false;
			}

			if (D.vblank_rise)
			{
				do_frame_fill = true;
				D.vblank_rise = false;
			}

			if (do_frame_fill)
			{
				FillVideoBuffer();
			}

			_islag = A._islag & B._islag & C._islag & (D._islag & !only_3_roms);

			if (_islag)
			{
				_lagcount++;
			}

			_frame++;

			return true;
		}

		// NOTE: this is HLE only and should be replaced eventually. It relies on information that a physical adapter does not have to sync transfers.
		// I think the adapter can see the high bit of each tranfer byte on its port, so is probably using that for syncing.
		// But, the details aren't known.
		public void do_frame_4x()
		{
			// advance one full frame
			for (int i = 0; i < 70224; i++)
			{
				A.do_single_step();
				B.do_single_step();
				C.do_single_step();
				D.do_single_step();

				x4_clock--;

				if (x4_clock == 0)
				{
					if (is_transmitting)
					{ 
						if (ready_to_transmit)
						{
							// fill the buffer on the second pass
							A.serialport.serial_clock = 1;
							A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);
							A.serialport.coming_in = (byte)((x4_buffer[transmit_byte + (buffer_parity ? 0x400 : 0)] >> bit_count) & 1);
							temp1_rec = (byte)((temp1_rec << 1) | A.serialport.going_out);

							if ((status_byte & 0x20) == 0x20)
							{
								B.serialport.serial_clock = 1;
								B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);
								B.serialport.coming_in = (byte)((x4_buffer[transmit_byte + (buffer_parity ? 0x400 : 0)] >> bit_count) & 1);

								temp2_rec = (byte)((temp2_rec << 1) | B.serialport.going_out);
							}
							else
							{
								temp2_rec = (byte)((temp2_rec << 1) | 0);
							}

							if ((status_byte & 0x40) == 0x40)
							{
								C.serialport.serial_clock = 1;
								C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);
								C.serialport.coming_in = (byte)((x4_buffer[transmit_byte + (buffer_parity ? 0x400 : 0)] >> bit_count) & 1);

								temp3_rec = (byte)((temp3_rec << 1) | C.serialport.going_out);
							}
							else
							{
								temp3_rec = (byte)((temp3_rec << 1) | 0);
							}

							if ((status_byte & 0x80) == 0x80)
							{
								D.serialport.serial_clock = 1;
								D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);
								D.serialport.coming_in = (byte)((x4_buffer[transmit_byte + (buffer_parity ? 0x400 : 0)] >> bit_count) & 1);

								temp4_rec = (byte)((temp4_rec << 1) | D.serialport.going_out);
							}
							else
							{
								temp4_rec = (byte)((temp4_rec << 1) | 0);
							}

							bit_count--;
							x4_clock = 512 + transmit_speed * 8;

							if (bit_count == -1)
							{
								bit_count = 7;
								x4_clock = 64;
								ready_to_transmit = false;

								if ((transmit_byte >= 1) && (transmit_byte < (num_bytes_transmit + 1)))
								{
									x4_buffer[(buffer_parity ? 0 : 0x400) + (transmit_byte - 1)] = temp1_rec;
									x4_buffer[(buffer_parity ? 0 : 0x400) + num_bytes_transmit + (transmit_byte - 1)] = temp2_rec;
									x4_buffer[(buffer_parity ? 0 : 0x400) + num_bytes_transmit * 2 + (transmit_byte - 1)] = temp3_rec;
									x4_buffer[(buffer_parity ? 0 : 0x400) + num_bytes_transmit * 3 + (transmit_byte - 1)] = temp4_rec;
								}

								//Console.WriteLine(temp1_rec + " " + temp2_rec + " " + temp3_rec + " " + temp4_rec + " " + transmit_byte);

								transmit_byte++;

								if (transmit_byte == num_bytes_transmit * 4)
								{
									transmit_byte = 0;
									buffer_parity = !buffer_parity;
								}
							}
						}
						else
						{
							if ((A.serialport.clk_rate == -1) && A.serialport.serial_control.Bit(7))
							{
								ready_to_transmit = true;

								if ((status_byte & 0x20) == 0x20)
								{
									if (!((B.serialport.clk_rate == -1) && B.serialport.serial_control.Bit(7))) { ready_to_transmit = false; }
								}
								if ((status_byte & 0x40) == 0x40)
								{
									if (!((C.serialport.clk_rate == -1) && C.serialport.serial_control.Bit(7))) { ready_to_transmit = false; }
								}
								if ((status_byte & 0x80) == 0x80)
								{
									if (!((D.serialport.clk_rate == -1) && D.serialport.serial_control.Bit(7))) { ready_to_transmit = false; }
								}
							}

							if (ready_to_transmit)
							{
								x4_clock = 512 + transmit_speed * 8;
							}
							else
							{
								x4_clock = 64;
							}
						}
					}
					else if (is_pinging)
					{
						if (ping_byte == 0)
						{
							// first byte sent is 0xFE
							if (ping_player == 1)
							{
								if ((A.serialport.clk_rate == -1) && A.serialport.serial_control.Bit(7))
								{
									A.serialport.serial_clock = 1;
									A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);
									A.serialport.coming_in = (byte)((0xFE >> bit_count) & 1);
								}

								received_byte |= (byte)(A.serialport.going_out << bit_count);
							}
							else if (ping_player == 2)
							{
								if ((B.serialport.clk_rate == -1) && B.serialport.serial_control.Bit(7))
								{
									B.serialport.serial_clock = 1;
									B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);
									B.serialport.coming_in = (byte)((0xFE >> bit_count) & 1);
								}

								received_byte |= (byte)(B.serialport.going_out << bit_count);
							}
							else if (ping_player == 3)
							{
								if ((C.serialport.clk_rate == -1) && C.serialport.serial_control.Bit(7))
								{
									C.serialport.serial_clock = 1;
									C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);
									C.serialport.coming_in = (byte)((0xFE >> bit_count) & 1);
								}

								received_byte |= (byte)(C.serialport.going_out << bit_count);
							}
							else
							{
								if ((D.serialport.clk_rate == -1) && D.serialport.serial_control.Bit(7))
								{
									D.serialport.serial_clock = 1;
									D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);
									D.serialport.coming_in = (byte)((0xFE >> bit_count) & 1);
								}

								received_byte |= (byte)(D.serialport.going_out << bit_count);
							}

							bit_count--;
							x4_clock = 512;

							if (bit_count == -1)
							{
								// player one can start the transmission phase
								if (ping_player == 1)
								{
									begin_transmitting_cnt = 0;
									num_bytes_transmit = received_byte;
								}

								//Console.WriteLine(ping_player + " " + ping_byte + " " + status_byte + " " + received_byte);

								bit_count = 7;
								received_byte = 0;

								ping_byte++;
								x4_clock = 64;
								is_pinging = false;
							}
						}
						else
						{
							// the next 3 bytes are the status byte (which may be updated in between each transfer)
							if (ping_player == 1)
							{
								if ((A.serialport.clk_rate == -1) && A.serialport.serial_control.Bit(7))
								{
									A.serialport.serial_clock = 1;
									A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);
									A.serialport.coming_in = (byte)((status_byte >> bit_count) & 1);
								}

								received_byte |= (byte)(A.serialport.going_out << bit_count);
							}
							else if (ping_player == 2)
							{
								if ((B.serialport.clk_rate == -1) && B.serialport.serial_control.Bit(7))
								{
									B.serialport.serial_clock = 1;
									B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);
									B.serialport.coming_in = (byte)((status_byte >> bit_count) & 1);
								}

								received_byte |= (byte)(B.serialport.going_out << bit_count);
							}
							else if (ping_player == 3)
							{
								if ((C.serialport.clk_rate == -1) && C.serialport.serial_control.Bit(7))
								{
									C.serialport.serial_clock = 1;
									C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);
									C.serialport.coming_in = (byte)((status_byte >> bit_count) & 1);
								}

								received_byte |= (byte)(C.serialport.going_out << bit_count);
							}
							else
							{
								if ((D.serialport.clk_rate == -1) && D.serialport.serial_control.Bit(7))
								{
									D.serialport.serial_clock = 1;
									D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);
									D.serialport.coming_in = (byte)((status_byte >> bit_count) & 1);
								}

								received_byte |= (byte)(D.serialport.going_out << bit_count);
							}

							bit_count--;
							x4_clock = 512;

							if (bit_count == -1)
							{
								is_pinging = false;
								x4_clock = 64;

								// player one can start the transmission phase
								if ((received_byte == 0xAA) && (ping_player == 1))
								{
									begin_transmitting_cnt += 1;

									if ((begin_transmitting_cnt >= 1) && (ping_byte == 3))
									{
										pre_transmit = true;
										is_pinging = false;
										ready_to_transmit = false;
										transmit_byte = 0;
										bit_count = 7;
									}
								}

								if (((received_byte & 0x88) == 0x88) && (ping_byte <= 2))
								{
									status_byte |= (byte)(1 << (3 + ping_player));
								}

								if ((ping_player == 1) && (ping_byte == 3) && !pre_transmit)
								{
									transmit_speed = received_byte;
								}

								//Console.WriteLine(ping_player + " " + ping_byte + " " + status_byte + " " + received_byte);

								bit_count = 7;
								received_byte = 0;

								ping_byte++;

								if (ping_byte == 4)
								{
									ping_byte = 0;
									ping_player++;

									if (ping_player == 5) { ping_player = 1; }

									begin_transmitting_cnt = 0;

									status_byte &= 0xF0;
									status_byte |= (byte)ping_player;

									time_out_check = true;
									x4_clock = 64;
								}
							}
						}
					}
					else if (pre_transmit)
					{
						if (ready_to_transmit)
						{
							// send four byte of 0xCC to signal start of transmitting

							// fill the buffer
							A.serialport.serial_clock = 1;
							A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);
							A.serialport.coming_in = (byte)((0xCC >> bit_count) & 1);

							if ((status_byte & 0x20) == 0x20)
							{
								B.serialport.serial_clock = 1;
								B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);
								B.serialport.coming_in = (byte)((0xCC >> bit_count) & 1);
							}

							if ((status_byte & 0x40) == 0x40)
							{
								C.serialport.serial_clock = 1;
								C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);
								C.serialport.coming_in = (byte)((0xCC >> bit_count) & 1);
							}

							if ((status_byte & 0x80) == 0x80)
							{
								D.serialport.serial_clock = 1;
								D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);
								D.serialport.coming_in = (byte)((0xCC >> bit_count) & 1);
							}

							bit_count--;
							x4_clock = 512;

							if (bit_count == -1)
							{
								bit_count = 7;
								x4_clock = 64;
								ready_to_transmit = false;

								transmit_byte++;

								if (transmit_byte == 4)
								{
									pre_transmit = false;
									is_transmitting = true;
									transmit_byte = 0;
									buffer_parity = false;
								}
							}
						}
						else
						{
							if ((A.serialport.clk_rate == -1) && A.serialport.serial_control.Bit(7))
							{
								ready_to_transmit = true;

								if ((status_byte & 0x20) == 0x20)
								{
									if (!((B.serialport.clk_rate == -1) && B.serialport.serial_control.Bit(7))) { ready_to_transmit = false; }
								}
								if ((status_byte & 0x40) == 0x40)
								{
									if (!((C.serialport.clk_rate == -1) && C.serialport.serial_control.Bit(7))) { ready_to_transmit = false; }
								}
								if ((status_byte & 0x80) == 0x80)
								{
									if (!((D.serialport.clk_rate == -1) && D.serialport.serial_control.Bit(7))) { ready_to_transmit = false; }
								}
							}

							if (ready_to_transmit)
							{
								x4_clock = 512;
							}
							else
							{
								x4_clock = 64;
							}
						}
					}
					else
					{
						x4_clock = 64;

						// wiat for a gameboy to request a ping. Timeout and go to the next one if nothing happening for some time.
						if ((ping_player == 1) && ((A.serialport.serial_control & 0x81) == 0x80))
						{
							is_pinging = true;
							x4_clock = 512;
							time_out_check = false;
						}
						else if ((ping_player == 2) && ((B.serialport.serial_control & 0x81) == 0x80))
						{
							is_pinging = true;
							x4_clock = 512;
							time_out_check = false;
						}
						else if ((ping_player == 3) && ((C.serialport.serial_control & 0x81) == 0x80))
						{
							is_pinging = true;
							x4_clock = 512;
							time_out_check = false;
						}
						else if ((ping_player == 4) && ((D.serialport.serial_control & 0x81) == 0x80))
						{
							is_pinging = true;
							x4_clock = 512;
							time_out_check = false;
						}

						if (time_out_check)
						{
							ping_player++;

							if (ping_player == 5) { ping_player = 1; }

							status_byte &= 0xF0;
							status_byte |= (byte)ping_player;

							x4_clock = 64;
						}
					}
				}
			}
		}

		public void do_frame_2x2()
		{
			// advance one full frame
			for (int i = 0; i < 70224; i++)
			{
				A.do_single_step();
				B.do_single_step();
				C.do_single_step();
				D.do_single_step();

				if (_cableconnected_UD)
				{
					// the signal to shift out a bit is when serial_clock = 1
					if (((A.serialport.serial_clock == 1) || (A.serialport.serial_clock == 2)) && (A.serialport.clk_rate > 0) && !do_2_next_1)
					{
						A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);

						if ((B.serialport.clk_rate == -1) && A.serialport.can_pulse)
						{
							B.serialport.serial_clock = A.serialport.serial_clock;
							B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);
							B.serialport.coming_in = A.serialport.going_out;
						}

						A.serialport.coming_in = B.serialport.going_out;
						A.serialport.can_pulse = false;
					}
					else if (((B.serialport.serial_clock == 1) || (B.serialport.serial_clock == 2)) && (B.serialport.clk_rate > 0))
					{
						do_2_next_1 = false;

						B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);

						if ((A.serialport.clk_rate == -1) && B.serialport.can_pulse)
						{
							A.serialport.serial_clock = B.serialport.serial_clock;
							A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);
							A.serialport.coming_in = B.serialport.going_out;
						}

						B.serialport.coming_in = A.serialport.going_out;
						B.serialport.can_pulse = false;

						if (B.serialport.serial_clock == 2) { do_2_next_1 = true; }
					}
					else
					{
						do_2_next_1 = false;
					}

					// the signal to shift out a bit is when serial_clock = 1
					if (((C.serialport.serial_clock == 1) || (C.serialport.serial_clock == 2)) && (C.serialport.clk_rate > 0) && !do_2_next_2)
					{
						C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);

						if ((D.serialport.clk_rate == -1) && C.serialport.can_pulse)
						{
							D.serialport.serial_clock = C.serialport.serial_clock;
							D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);
							D.serialport.coming_in = C.serialport.going_out;
						}

						C.serialport.coming_in = D.serialport.going_out;
						C.serialport.can_pulse = false;
					}
					else if (((D.serialport.serial_clock == 1) || (D.serialport.serial_clock == 2)) && (D.serialport.clk_rate > 0))
					{
						do_2_next_2 = false;

						D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);

						if ((C.serialport.clk_rate == -1) && D.serialport.can_pulse)
						{
							C.serialport.serial_clock = D.serialport.serial_clock;
							C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);
							C.serialport.coming_in = D.serialport.going_out;
						}

						D.serialport.coming_in = C.serialport.going_out;
						D.serialport.can_pulse = false;

						if (D.serialport.serial_clock == 2) { do_2_next_2 = true; }
					}
					else
					{
						do_2_next_2 = false;
					}
				}
				else if (_cableconnected_LR)
				{
					// the signal to shift out a bit is when serial_clock = 1
					if (((A.serialport.serial_clock == 1) || (A.serialport.serial_clock == 2)) && (A.serialport.clk_rate > 0) && !do_2_next_1)
					{
						A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);

						if ((D.serialport.clk_rate == -1) && A.serialport.can_pulse)
						{
							D.serialport.serial_clock = A.serialport.serial_clock;
							D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);
							D.serialport.coming_in = A.serialport.going_out;
						}

						A.serialport.coming_in = D.serialport.going_out;
						A.serialport.can_pulse = false;
					}
					else if (((D.serialport.serial_clock == 1) || (D.serialport.serial_clock == 2)) && (D.serialport.clk_rate > 0))
					{
						do_2_next_1 = false;

						D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);

						if ((A.serialport.clk_rate == -1) && D.serialport.can_pulse)
						{
							A.serialport.serial_clock = D.serialport.serial_clock;
							A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);
							A.serialport.coming_in = D.serialport.going_out;
						}

						D.serialport.coming_in = A.serialport.going_out;
						D.serialport.can_pulse = false;

						if (D.serialport.serial_clock == 2) { do_2_next_1 = true; }
					}
					else
					{
						do_2_next_1 = false;
					}

					// the signal to shift out a bit is when serial_clock = 1
					if (((B.serialport.serial_clock == 1) || (B.serialport.serial_clock == 2)) && (B.serialport.clk_rate > 0) && !do_2_next_2)
					{
						B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);

						if ((C.serialport.clk_rate == -1) && B.serialport.can_pulse)
						{
							C.serialport.serial_clock = B.serialport.serial_clock;
							C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);
							C.serialport.coming_in = B.serialport.going_out;
						}

						B.serialport.coming_in = C.serialport.going_out;
						B.serialport.can_pulse = false;
					}
					else if (((C.serialport.serial_clock == 1) || (C.serialport.serial_clock == 2)) && (C.serialport.clk_rate > 0))
					{
						do_2_next_2 = false;

						C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);

						if ((B.serialport.clk_rate == -1) && C.serialport.can_pulse)
						{
							B.serialport.serial_clock = C.serialport.serial_clock;
							B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);
							B.serialport.coming_in = C.serialport.going_out;
						}

						C.serialport.coming_in = B.serialport.going_out;
						C.serialport.can_pulse = false;

						if (C.serialport.serial_clock == 2) { do_2_next_2 = true; }
					}
					else
					{
						do_2_next_2 = false;
					}
				}
				else if (_cableconnected_X)
				{
					// the signal to shift out a bit is when serial_clock = 1
					if (((C.serialport.serial_clock == 1) || (C.serialport.serial_clock == 2)) && (C.serialport.clk_rate > 0) && !do_2_next_1)
					{
						C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);

						if ((A.serialport.clk_rate == -1) && C.serialport.can_pulse)
						{
							A.serialport.serial_clock = C.serialport.serial_clock;
							A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);
							A.serialport.coming_in = C.serialport.going_out;
						}

						C.serialport.coming_in = A.serialport.going_out;
						C.serialport.can_pulse = false;
					}
					else if (((A.serialport.serial_clock == 1) || (A.serialport.serial_clock == 2)) && (A.serialport.clk_rate > 0))
					{
						do_2_next_1 = false;

						A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);

						if ((C.serialport.clk_rate == -1) && A.serialport.can_pulse)
						{
							C.serialport.serial_clock = A.serialport.serial_clock;
							C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);
							C.serialport.coming_in = A.serialport.going_out;
						}

						A.serialport.coming_in = C.serialport.going_out;
						A.serialport.can_pulse = false;

						if (A.serialport.serial_clock == 2) { do_2_next_1 = true; }
					}
					else
					{
						do_2_next_1 = false;
					}

					// the signal to shift out a bit is when serial_clock = 1
					if (((B.serialport.serial_clock == 1) || (B.serialport.serial_clock == 2)) && (B.serialport.clk_rate > 0) && !do_2_next_2)
					{
						B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);

						if ((D.serialport.clk_rate == -1) && B.serialport.can_pulse)
						{
							D.serialport.serial_clock = B.serialport.serial_clock;
							D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);
							D.serialport.coming_in = B.serialport.going_out;
						}

						B.serialport.coming_in = D.serialport.going_out;
						B.serialport.can_pulse = false;
					}
					else if (((D.serialport.serial_clock == 1) || (D.serialport.serial_clock == 2)) && (D.serialport.clk_rate > 0))
					{
						do_2_next_2 = false;

						D.serialport.going_out = (byte)(D.serialport.serial_data >> 7);

						if ((B.serialport.clk_rate == -1) && D.serialport.can_pulse)
						{
							B.serialport.serial_clock = D.serialport.serial_clock;
							B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);
							B.serialport.coming_in = D.serialport.going_out;
						}

						D.serialport.coming_in = B.serialport.going_out;
						D.serialport.can_pulse = false;

						if (D.serialport.serial_clock == 2) { do_2_next_2 = true; }
					}
					else
					{
						do_2_next_2 = false;
					}
				}
			}

			// clear the screens as needed
			if (A.ppu.clear_screen)
			{
				A.clear_screen_func();
				do_frame_fill = true;
			}

			if (B.ppu.clear_screen)
			{
				B.clear_screen_func();
				do_frame_fill = true;
			}

			if (C.ppu.clear_screen)
			{
				C.clear_screen_func();
				do_frame_fill = true;
			}

			if (D.ppu.clear_screen)
			{
				D.clear_screen_func();
				do_frame_fill = true;
			}
		}

		public void do_frame_3()
		{
			// advance one full frame
			for (int i = 0; i < 70224; i++)
			{
				A.do_single_step();
				B.do_single_step();
				C.do_single_step();
				
				if (_cableconnected_UD)
				{
					// the signal to shift out a bit is when serial_clock = 1
					if (((A.serialport.serial_clock == 1) || (A.serialport.serial_clock == 2)) && (A.serialport.clk_rate > 0) && !do_2_next_1)
					{
						A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);

						if ((C.serialport.clk_rate == -1) && A.serialport.can_pulse)
						{
							C.serialport.serial_clock = A.serialport.serial_clock;
							C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);
							C.serialport.coming_in = A.serialport.going_out;
						}

						A.serialport.coming_in = C.serialport.going_out;
						A.serialport.can_pulse = false;
					}
					else if (((C.serialport.serial_clock == 1) || (C.serialport.serial_clock == 2)) && (C.serialport.clk_rate > 0))
					{
						do_2_next_1 = false;

						C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);

						if ((A.serialport.clk_rate == -1) && C.serialport.can_pulse)
						{
							A.serialport.serial_clock = C.serialport.serial_clock;
							A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);
							A.serialport.coming_in = C.serialport.going_out;
						}

						C.serialport.coming_in = A.serialport.going_out;
						C.serialport.can_pulse = false;

						if (C.serialport.serial_clock == 2) { do_2_next_1 = true; }
					}
					else
					{
						do_2_next_1 = false;
					}

					// do IR transfer
					if (A.IR_write > 0)
					{
						A.IR_write--;
						if (A.IR_write == 0)
						{
							C.IR_receive = A.IR_signal;
							if ((C.IR_self & C.IR_receive) == 2) { C.IR_reg |= 2; }
							else { C.IR_reg &= 0xFD; }
							if ((A.IR_self & A.IR_receive) == 2) { A.IR_reg |= 2; }
							else { A.IR_reg &= 0xFD; }
						}
					}

					if (C.IR_write > 0)
					{
						C.IR_write--;
						if (C.IR_write == 0)
						{
							A.IR_receive = C.IR_signal;
							if ((A.IR_self & A.IR_receive) == 2) { A.IR_reg |= 2; }
							else { A.IR_reg &= 0xFD; }
							if ((C.IR_self & C.IR_receive) == 2) { C.IR_reg |= 2; }
							else { C.IR_reg &= 0xFD; }
						}
					}
				}
				else if (_cableconnected_X)
				{
					// the signal to shift out a bit is when serial_clock = 1
					if (((C.serialport.serial_clock == 1) || (C.serialport.serial_clock == 2)) && (C.serialport.clk_rate > 0) && !do_2_next_1)
					{
						C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);

						if ((B.serialport.clk_rate == -1) && C.serialport.can_pulse)
						{
							B.serialport.serial_clock = C.serialport.serial_clock;
							B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);
							B.serialport.coming_in = C.serialport.going_out;
						}

						C.serialport.coming_in = B.serialport.going_out;
						C.serialport.can_pulse = false;
					}
					else if (((B.serialport.serial_clock == 1) || (B.serialport.serial_clock == 2)) && (B.serialport.clk_rate > 0))
					{
						do_2_next_1 = false;

						B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);

						if ((C.serialport.clk_rate == -1) && B.serialport.can_pulse)
						{
							C.serialport.serial_clock = B.serialport.serial_clock;
							C.serialport.going_out = (byte)(C.serialport.serial_data >> 7);
							C.serialport.coming_in = B.serialport.going_out;
						}

						B.serialport.coming_in = C.serialport.going_out;
						B.serialport.can_pulse = false;

						if (B.serialport.serial_clock == 2) { do_2_next_1 = true; }
					}
					else
					{
						do_2_next_1 = false;
					}

					// do IR transfer
					if (C.IR_write > 0)
					{
						C.IR_write--;
						if (C.IR_write == 0)
						{
							B.IR_receive = C.IR_signal;
							if ((B.IR_self & B.IR_receive) == 2) { B.IR_reg |= 2; }
							else { B.IR_reg &= 0xFD; }
							if ((C.IR_self & C.IR_receive) == 2) { C.IR_reg |= 2; }
							else { C.IR_reg &= 0xFD; }
						}
					}

					if (B.IR_write > 0)
					{
						B.IR_write--;
						if (B.IR_write == 0)
						{
							C.IR_receive = B.IR_signal;
							if ((C.IR_self & C.IR_receive) == 2) { C.IR_reg |= 2; }
							else { C.IR_reg &= 0xFD; }
							if ((B.IR_self & B.IR_receive) == 2) { B.IR_reg |= 2; }
							else { B.IR_reg &= 0xFD; }
						}
					}
				}
				else if (_cableconnected_LR)
				{
					// the signal to shift out a bit is when serial_clock = 1
					if (((B.serialport.serial_clock == 1) || (B.serialport.serial_clock == 2)) && (B.serialport.clk_rate > 0) && !do_2_next_1)
					{
						B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);

						if ((A.serialport.clk_rate == -1) && B.serialport.can_pulse)
						{
							A.serialport.serial_clock = B.serialport.serial_clock;
							A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);
							A.serialport.coming_in = B.serialport.going_out;
						}

						B.serialport.coming_in = A.serialport.going_out;
						B.serialport.can_pulse = false;
					}
					else if (((A.serialport.serial_clock == 1) || (A.serialport.serial_clock == 2)) && (A.serialport.clk_rate > 0))
					{
						do_2_next_1 = false;

						A.serialport.going_out = (byte)(A.serialport.serial_data >> 7);

						if ((B.serialport.clk_rate == -1) && A.serialport.can_pulse)
						{
							B.serialport.serial_clock = A.serialport.serial_clock;
							B.serialport.going_out = (byte)(B.serialport.serial_data >> 7);
							B.serialport.coming_in = A.serialport.going_out;
						}

						A.serialport.coming_in = B.serialport.going_out;
						A.serialport.can_pulse = false;

						if (A.serialport.serial_clock == 2) { do_2_next_1 = true; }
					}
					else
					{
						do_2_next_1 = false;
					}

					// do IR transfer
					if (B.IR_write > 0)
					{
						B.IR_write--;
						if (B.IR_write == 0)
						{
							A.IR_receive = B.IR_signal;
							if ((A.IR_self & A.IR_receive) == 2) { A.IR_reg |= 2; }
							else { A.IR_reg &= 0xFD; }
							if ((B.IR_self & B.IR_receive) == 2) { B.IR_reg |= 2; }
							else { B.IR_reg &= 0xFD; }
						}
					}

					if (A.IR_write > 0)
					{
						A.IR_write--;
						if (A.IR_write == 0)
						{
							B.IR_receive = A.IR_signal;
							if ((B.IR_self & B.IR_receive) == 2) { B.IR_reg |= 2; }
							else { B.IR_reg &= 0xFD; }
							if ((A.IR_self & A.IR_receive) == 2) { A.IR_reg |= 2; }
							else { A.IR_reg &= 0xFD; }
						}
					}
				}
			}

			// clear the screens as needed
			if (A.ppu.clear_screen)
			{
				A.clear_screen_func();
				do_frame_fill = true;
			}

			if (C.ppu.clear_screen)
			{
				C.clear_screen_func();
				do_frame_fill = true;
			}

			if (B.ppu.clear_screen)
			{
				B.clear_screen_func();
				do_frame_fill = true;
			}
		}

		public void GetControllerState(IController controller)
		{
			InputCallbacks.Call();
			A.multi_core_controller_byte = _controllerDeck.ReadPort1(controller);
			B.multi_core_controller_byte = _controllerDeck.ReadPort2(controller);
			C.multi_core_controller_byte = _controllerDeck.ReadPort3(controller);
			D.multi_core_controller_byte = _controllerDeck.ReadPort4(controller);
		}

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.GBL;

		public bool DeterministicEmulation { get; set; }

		public void ResetCounters()
		{
			_frame = 0;
			_lagcount = 0;
			_islag = false;
		}

		public void Dispose()
		{
			A.Dispose();
			B.Dispose();
			C.Dispose();
			D.Dispose();
		}

		public int[] _vidbuffer = new int[160 * 2 * 144 * 2];

		public int[] GetVideoBuffer()
		{
			return _vidbuffer;		
		}

		public void FillVideoBuffer()
		{
			// combine all the video buffers, depending on # of roms
			if (only_3_roms)
			{
				for (int i = 0; i < 144; i++)
				{
					for (int j = 0; j < 160; j++)
					{
						_vidbuffer[i * 320 + j] = A.frame_buffer[i * 160 + j];				
						_vidbuffer[i * 320 + j + 160] = B.frame_buffer[i * 160 + j];
						_vidbuffer[(i + 144) * 320 + j + 80] = C.frame_buffer[i * 160 + j];
					}
				}
			}
			else
			{
				for (int i = 0; i < 144; i++)
				{
					for (int j = 0; j < 160; j++)
					{
						_vidbuffer[i * 320 + j] = A.frame_buffer[i * 160 + j];
						_vidbuffer[(i + 144) * 320 + j] = B.frame_buffer[i * 160 + j];
						_vidbuffer[(i + 144) * 320 + j + 160] = C.frame_buffer[i * 160 + j];
						_vidbuffer[i * 320 + j + 160] = D.frame_buffer[i * 160 + j];
					}
				}
			}		
		}

		public int VirtualWidth => 160 * 2;
		public int VirtualHeight => 144 * 2;
		public int BufferWidth => 160 * 2;
		public int BufferHeight => 144 * 2;
		public int BackgroundColor => unchecked((int)0xFF000000);
		public int VsyncNumerator => 262144;
		public int VsyncDenominator => 4389;

		public static readonly uint[] color_palette_BW = { 0xFFFFFFFF , 0xFFAAAAAA, 0xFF555555, 0xFF000000 };
		public static readonly uint[] color_palette_Gr = { 0xFFA4C505, 0xFF88A905, 0xFF1D551D, 0xFF052505 };

		public uint[] color_palette = new uint[4];

		public bool CanProvideAsync => false;

		public void SetSyncMode(SyncSoundMode mode)
		{
			if (mode != SyncSoundMode.Sync)
			{
				throw new InvalidOperationException("Only Sync mode is supported_");
			}
		}

		public SyncSoundMode SyncMode => SyncSoundMode.Sync;

		public void GetSamplesSync(out short[] samples, out int nsamp)
		{
			A.audio.GetSamplesSync(out var temp_samp_A, out var nsamp_A);
			B.audio.GetSamplesSync(out var temp_samp_B, out var nsamp_B);
			C.audio.GetSamplesSync(out var temp_samp_C, out var nsamp_C);
			D.audio.GetSamplesSync(out var temp_samp_D, out var nsamp_D);

			if (Link4xSettings.AudioSet == GBLink4xSettings.AudioSrc.A)
			{
				samples = temp_samp_A;
				nsamp = nsamp_A;
			}
			else if (Link4xSettings.AudioSet == GBLink4xSettings.AudioSrc.B)
			{
				samples = temp_samp_B;
				nsamp = nsamp_B;
			}
			else if (Link4xSettings.AudioSet == GBLink4xSettings.AudioSrc.C)
			{
				samples = temp_samp_C;
				nsamp = nsamp_C;
			}
			else if (Link4xSettings.AudioSet == GBLink4xSettings.AudioSrc.D)
			{
				samples = temp_samp_D;
				nsamp = nsamp_D;
			}
			else
			{
				samples = new short[0];
				nsamp = 0;
			}
		}

		public void GetSamplesAsync(short[] samples)
		{
			throw new NotSupportedException("Async is not available");
		}

		public void DiscardSamples()
		{
			A.audio.DiscardSamples();
			B.audio.DiscardSamples();
			C.audio.DiscardSamples();
			D.audio.DiscardSamples();
		}

		private void GetSamples(short[] samples)
		{

		}

		public void DisposeSound()
		{
			A.audio.DisposeSound();
			B.audio.DisposeSound();
			C.audio.DisposeSound();
			D.audio.DisposeSound();
		}
	}
}
