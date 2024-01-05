using System;

using BizHawk.Common;
using BizHawk.Emulation.Common;
using BizHawk.Common.NumberExtensions;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	/*
		Sound Emulation
		NOTES: 

		what happens with unused bits and registers in the sound area? (for now assume all bits writable.)

		are all assigned bits readable? (for now assume yes.)

		does wave ram bank bit change when playing bank changes in 64 bit mode? (for now assume no.)

		does the written bank change based on which bank is getting played back, or does it always follow the bank bit? (for now assume bank bit.)

		does writing to any FIFO data reg automatically increase the FIFO pointer by 4 bytes? (For now assume yes.)

		Are the FIFO's cleared when disabling sound or only on reset? (For now clear.)

		What does FIFO volume setting 3 do? (For now assume %100)

		Is there an underlying counter used by all timers and the sound engine? (For now assume sound is seperate.)
	*/

	public partial class GBAHawk_Debug : ISoundProvider
	{

		public static bool[] snd_Duty_Cycles = { false, false, false, false, false, false, false, true,
												 true, false, false, false, false, false, false, true,
												 true, false, false, false, false, true, true, true,
												 false, true, true, true, true, true, true, false };

		public static int[] snd_Divisor = { 8, 16, 32, 48, 64, 80, 96, 112 };

		public static int[] snd_Chan_Mult_Table = { 1, 2, 4, 4 };

		public byte[] snd_Audio_Regs = new byte[0x30];

		public byte[] snd_Wave_RAM = new byte[32];

		public int snd_Wave_Decay_cnt;
		public int snd_Internal_cnt;
		public int snd_Divider;
		public bool snd_Wave_Decay_Done;

		// Audio Variables
		// derived
		public bool														snd_WAVE_DAC_pow;
		public bool																					snd_NOISE_wdth_md;
		public bool snd_SQ1_negate;
		public bool snd_SQ1_trigger,		snd_SQ2_trigger,			snd_WAVE_trigger,			snd_NOISE_trigger;
		public bool snd_SQ1_len_en,			snd_SQ2_len_en,				snd_WAVE_len_en,			snd_NOISE_len_en;
		public bool snd_SQ1_env_add,		snd_SQ2_env_add,										snd_NOISE_env_add;
		public byte														snd_WAVE_vol_code;
		public byte																					snd_NOISE_clk_shft;
		public byte																					snd_NOISE_div_code;
		public byte snd_SQ1_shift;
		public byte snd_SQ1_duty,			snd_SQ2_duty;
		public byte snd_SQ1_st_vol,			snd_SQ2_st_vol,											snd_NOISE_st_vol;
		public byte snd_SQ1_per,			snd_SQ2_per,											snd_NOISE_per;
		public byte snd_SQ1_swp_prd;
		public int snd_SQ1_frq,				snd_SQ2_frq,				snd_WAVE_frq;
		public ushort snd_SQ1_length,		snd_SQ2_length,				snd_WAVE_length,			snd_NOISE_length;
		// state
		public bool														snd_WAVE_can_get;
		public bool snd_SQ1_calc_done;
		public bool snd_SQ1_swp_enable;
		public bool snd_SQ1_vol_done,		snd_SQ2_vol_done,										snd_NOISE_vol_done;
		public bool snd_SQ1_enable,			snd_SQ2_enable,				snd_WAVE_enable,			snd_NOISE_enable;
		public byte snd_SQ1_vol_state,		snd_SQ2_vol_state,										snd_NOISE_vol_state;
		public byte snd_SQ1_duty_cntr,		snd_SQ2_duty_cntr;
		public byte														snd_WAVE_wave_cntr;
		public int snd_SQ1_frq_shadow;
		public int snd_SQ1_intl_cntr,		snd_SQ2_intl_cntr,			snd_WAVE_intl_cntr,			snd_NOISE_intl_cntr;
		public int snd_SQ1_vol_per,			snd_SQ2_vol_per,										snd_NOISE_vol_per;
		public int snd_SQ1_intl_swp_cnt;
		public int																					snd_NOISE_LFSR;
		public ushort snd_SQ1_len_cntr,		snd_SQ2_len_cntr,			snd_WAVE_len_cntr,			snd_NOISE_len_cntr;

		// GBA specific registers
		public int snd_Wave_Bank, snd_Wave_Bank_Playing;
		public int snd_Chan_Mult;
		public byte snd_CTRL_GBA_Low, snd_CTRL_GBA_High;
		public byte snd_Bias_Low, snd_Bias_High;
		public bool snd_Wave_Size;
		public bool snd_Wave_Vol_Force;

		// FIFOs
		public int snd_FIFO_A_ptr, snd_FIFO_B_ptr;
		public int snd_FIFO_A_Timer, snd_FIFO_B_Timer;
		public int snd_FIFO_A_Output, snd_FIFO_B_Output;
		public int snd_FIFO_A_Mult, snd_FIFO_B_Mult;

		public byte[] snd_FIFO_A = new byte[32];
		public byte[] snd_FIFO_B = new byte[32];

		public byte[] snd_FIFO_A_Data = new byte[4];
		public byte[] snd_FIFO_B_Data = new byte[4];

		public  byte snd_FIFO_A_Sample, snd_FIFO_B_Sample;

		public bool snd_FIFO_A_Tick, snd_FIFO_B_Tick;
		public bool snd_FIFO_A_Enable_L, snd_FIFO_B_Enable_L, snd_FIFO_A_Enable_R, snd_FIFO_B_Enable_R;

		// computed
		public int snd_SQ1_output, snd_SQ2_output, snd_WAVE_output, snd_NOISE_output;

		// Contol Variables
		public bool snd_CTRL_sq1_L_en;
		public bool snd_CTRL_sq2_L_en;
		public bool snd_CTRL_wave_L_en;
		public bool snd_CTRL_noise_L_en;
		public bool snd_CTRL_sq1_R_en;
		public bool snd_CTRL_sq2_R_en;
		public bool snd_CTRL_wave_R_en;
		public bool snd_CTRL_noise_R_en;
		public bool snd_CTRL_power;
		public byte snd_CTRL_vol_L;
		public byte snd_CTRL_vol_R;

		public int snd_Sequencer_len, snd_Sequencer_vol, snd_Sequencer_swp;
		public int snd_Sequencer_reset_cd;

		public byte snd_Sample;

		public uint snd_Master_Clock;

		public int snd_Latched_Sample_L, snd_Latched_Sample_R;

		public int snd_BIAS_Offset, snd_Sample_Rate;

		public byte snd_Read_Reg_8(uint addr)
		{
			byte ret = 0;

			if (addr < 0x8C)
			{
				ret = snd_Audio_Regs[addr - 0x60];
			}
			else if ((addr < 0xA0) && (addr >= 0x90))
			{
				int ofst = (int)(snd_Wave_Bank + addr - 0x90);

				ret = snd_Wave_RAM[ofst];			
			}
			else
			{
				// FIFO not readable, other addresses are open bus
				ret = (byte)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); // open bus;
			}

			return ret;
		}

		public ushort snd_Read_Reg_16(uint addr)
		{
			ushort ret = 0;

			if (addr < 0x8C)
			{
				ret = (ushort)((snd_Audio_Regs[addr - 0x60 + 1] << 8) | snd_Audio_Regs[addr - 0x60]);
			}
			else if ((addr < 0xA0) && (addr >= 0x90))
			{
				int ofst = (int)(snd_Wave_Bank + (addr - 0x90));

				ret = (ushort)((snd_Wave_RAM[ofst + 1] << 8) | snd_Wave_RAM[ofst]);
			}
			else
			{
				// FIFO not readable, other addresses are open bus
				ret = (ushort)(cpu_Last_Bus_Value & 0xFFFF); // open bus
			}

			return ret;
		}

		public uint snd_Read_Reg_32(uint addr)
		{
			uint ret = 0;

			if (addr < 0x8C)
			{
				ret = (uint)((snd_Audio_Regs[addr - 0x60 + 3] << 24) | (snd_Audio_Regs[addr - 0x60 + 2] << 16) | (snd_Audio_Regs[addr - 0x60 + 1] << 8) | snd_Audio_Regs[addr - 0x60]);
			}
			else if ((addr < 0xA0) && (addr >= 0x90))
			{
				int ofst = (int)(snd_Wave_Bank + (addr - 0x90));

				ret = (uint)((snd_Wave_RAM[ofst + 3] << 24) | (snd_Wave_RAM[ofst + 2] << 16) | (snd_Wave_RAM[ofst + 1] << 8) | snd_Wave_RAM[ofst]);
			}
			else
			{
				// FIFO not readable, other addresses are open bus
				ret = cpu_Last_Bus_Value; // open bus
			}

			return ret;
		}

		public void snd_Write_Reg_8(uint addr, byte value)
		{
			if (addr < 0x90)
			{
				snd_Update_Regs(addr, value);
			}
			else if (addr < 0xA0)
			{
				int ofst = (int)(snd_Wave_Bank + addr - 0x90);

				snd_Wave_RAM[ofst] = value;
			}
			else if (addr < 0xA4)
			{
				snd_FIFO_A_Data[(addr & 3)] = value;
				snd_Write_FIFO_Data(true);
			}
			else if (addr < 0xA8)
			{
				snd_FIFO_B_Data[(addr & 3)] = value;
				snd_Write_FIFO_Data(false);
			}
		}

		public void snd_Write_Reg_16(uint addr, ushort value)
		{
			if (addr < 0x90)
			{
				snd_Update_Regs(addr, (byte)(value & 0xFF));
				snd_Update_Regs((uint)(addr + 1), (byte)((value >> 8) & 0xFF));
			}
			else if (addr < 0xA0)
			{
				int ofst = (int)(snd_Wave_Bank + (addr - 0x90));

				snd_Wave_RAM[ofst] = (byte)(value & 0xFF);
				snd_Wave_RAM[ofst + 1] = (byte)((value >> 8) & 0xFF);
			}
			else if (addr < 0xA4)
			{
				if (addr == 0xA0)
				{
					snd_FIFO_A_Data[0] = (byte)(value & 0xFF);
					snd_FIFO_A_Data[1] = (byte)((value >> 8) & 0xFF);
					snd_Write_FIFO_Data(true);
				}
				else
				{
					snd_FIFO_A_Data[2] = (byte)(value & 0xFF);
					snd_FIFO_A_Data[3] = (byte)((value >> 8) & 0xFF);
					snd_Write_FIFO_Data(true);
				}
			}
			else if (addr < 0xA8)
			{
				if (addr == 0xA6)
				{
					snd_FIFO_B_Data[0] = (byte)(value & 0xFF);
					snd_FIFO_B_Data[1] = (byte)((value >> 8) & 0xFF);
					snd_Write_FIFO_Data(false);
				}
				else
				{
					snd_FIFO_B_Data[2] = (byte)(value & 0xFF);
					snd_FIFO_B_Data[3] = (byte)((value >> 8) & 0xFF);
					snd_Write_FIFO_Data(false);
				}
			}
		}

		public void snd_Write_Reg_32(uint addr, uint value)
		{
			if (addr < 0x90)
			{
				snd_Update_Regs(addr, (byte)(value & 0xFF));
				snd_Update_Regs((uint)(addr + 1), (byte)((value >> 8) & 0xFF));
				snd_Update_Regs((uint)(addr + 2), (byte)((value >> 16) & 0xFF));
				snd_Update_Regs((uint)(addr + 3), (byte)((value >> 24) & 0xFF));
			}
			else if (addr < 0xA0)
			{
				int ofst = (int)(snd_Wave_Bank + (addr - 0x90));

				snd_Wave_RAM[ofst] = (byte)(value & 0xFF);
				snd_Wave_RAM[ofst + 1] = (byte)((value >> 8) & 0xFF);
				snd_Wave_RAM[ofst + 2] = (byte)((value >> 16) & 0xFF);
				snd_Wave_RAM[ofst + 3] = (byte)((value >> 24) & 0xFF);
			}
			else if (addr < 0xA4)
			{
				snd_FIFO_A_Data[0] = (byte)(value & 0xFF);
				snd_FIFO_A_Data[1] = (byte)((value >> 8) & 0xFF);
				snd_FIFO_A_Data[2] = (byte)((value >> 16) & 0xFF);
				snd_FIFO_A_Data[3] = (byte)((value >> 24) & 0xFF);
				snd_Write_FIFO_Data(true);
			}
			else if (addr < 0xA8)
			{
				snd_FIFO_B_Data[0] = (byte)(value & 0xFF);
				snd_FIFO_B_Data[1] = (byte)((value >> 8) & 0xFF);
				snd_FIFO_B_Data[2] = (byte)((value >> 16) & 0xFF);
				snd_FIFO_B_Data[3] = (byte)((value >> 24) & 0xFF);
				snd_Write_FIFO_Data(false);
			}
		}

		public void snd_Write_FIFO_Data(bool chan_A)
		{
			//Console.WriteLine(CycleCount + " " + chan_A);
			if (snd_CTRL_power)
			{
				for (int i = 0; i < 4; i++)
				{
					if (chan_A)
					{
						if (snd_FIFO_A_ptr < 32)
						{
							snd_FIFO_A[snd_FIFO_A_ptr] = snd_FIFO_A_Data[i];
							snd_FIFO_A_ptr += 1;
						}

						if (snd_FIFO_A_ptr == 32)
						{
							snd_FIFO_A_ptr = 0;
						}
					}
					else
					{
						if (snd_FIFO_B_ptr < 32)
						{
							snd_FIFO_B[snd_FIFO_B_ptr] = snd_FIFO_B_Data[i];
							snd_FIFO_B_ptr += 1;
						}

						if (snd_FIFO_B_ptr == 32)
						{
							snd_FIFO_B_ptr = 0;
						}
					}
				}
			}
		}

		public void snd_Update_Regs(uint addr, byte value)
		{
			// while power is on, everything is writable
			//Console.WriteLine("write: " + (addr & 0xFF) + " " + value + " " + CycleCount);
			if (snd_CTRL_power)
			{
				if (addr < 0x90)
				{
					switch (addr)
					{
						case 0x60:												// NR10 (sweep)
							snd_SQ1_swp_prd = (byte)((value & 0x70) >> 4);
							snd_SQ1_negate = (value & 8) > 0;
							snd_SQ1_shift = (byte)(value & 7);

							if (!snd_SQ1_negate && snd_SQ1_calc_done) { snd_SQ1_enable = false; }

							value &= 0x7F;
							break;

						case 0x61:
							// not writable
							value = 0;
							break;

						case 0x62:												// NR11 (sound length / wave pattern duty %)
							snd_SQ1_duty = (byte)((value & 0xC0) >> 6);
							snd_SQ1_length = (ushort)(64 - (value & 0x3F));
							snd_SQ1_len_cntr = snd_SQ1_length;

							// lower bits not readable
							value &= 0xC0;
							break;

						case 0x63:												// NR12 (envelope)
							snd_SQ1_st_vol = (byte)((value & 0xF0) >> 4);
							snd_SQ1_env_add = (value & 8) > 0;
							snd_SQ1_per = (byte)(value & 7);

							// several glitchy effects happen when writing to snd_NRx2 during audio playing
							if (((snd_Audio_Regs[0x03] & 7) == 0) && !snd_SQ1_vol_done) { snd_SQ1_vol_state++; }
							else if ((snd_Audio_Regs[0x03] & 8) == 0) { snd_SQ1_vol_state += 2; }

							if (((snd_Audio_Regs[0x03] ^ value) & 8) > 0) { snd_SQ1_vol_state = (byte)(0x10 - snd_SQ1_vol_state); }

							snd_SQ1_vol_state &= 0xF;

							if ((value & 0xF8) == 0) { snd_SQ1_enable = snd_SQ1_swp_enable = false; }
							break;

						case 0x64:												// NR13 (freq low)
							snd_SQ1_frq &= 0x700;
							snd_SQ1_frq |= value;

							// not readable
							value = 0;
							break;

						case 0x65:												// NR14 (freq hi)
							snd_SQ1_trigger = (value & 0x80) > 0;
							snd_SQ1_frq &= 0xFF;
							snd_SQ1_frq |= (ushort)((value & 7) << 8);

							if (((snd_Sequencer_len & 1) == 0))
							{
								if (!snd_SQ1_len_en && ((value & 0x40) > 0) && (snd_SQ1_len_cntr > 0))
								{
									snd_SQ1_len_cntr--;
									if ((snd_SQ1_len_cntr == 0) && !snd_SQ1_trigger) { snd_SQ1_enable = snd_SQ1_swp_enable = false; }
								}
							}

							if (snd_SQ1_trigger)
							{
								snd_SQ1_enable = true;
								snd_SQ1_vol_done = false;
								snd_SQ1_duty_cntr = 0;

								if (snd_SQ1_len_cntr == 0)
								{
									snd_SQ1_len_cntr = 64;
									if (((value & 0x40) > 0) && ((snd_Sequencer_len & 1) == 0)) { snd_SQ1_len_cntr--; }
								}
								snd_SQ1_vol_state = snd_SQ1_st_vol;
								snd_SQ1_vol_per = (snd_SQ1_per > 0) ? snd_SQ1_per : 8;
								if (snd_Sequencer_vol == 4) { snd_SQ1_vol_per++; }
								snd_SQ1_frq_shadow = snd_SQ1_frq;
								snd_SQ1_intl_cntr = ((2048 - snd_SQ1_frq_shadow) * 4) | (snd_SQ1_intl_cntr & 3);

								snd_SQ1_intl_swp_cnt = snd_SQ1_swp_prd > 0 ? snd_SQ1_swp_prd : 8;
								snd_SQ1_calc_done = false;

								if ((snd_SQ1_shift > 0) || (snd_SQ1_swp_prd > 0))
								{
									snd_SQ1_swp_enable = true;
								}
								else
								{
									snd_SQ1_swp_enable = false;
								}

								if (snd_SQ1_shift > 0)
								{
									int shadow_frq = snd_SQ1_frq_shadow;
									shadow_frq = shadow_frq >> snd_SQ1_shift;
									if (snd_SQ1_negate) { shadow_frq = -shadow_frq; }
									shadow_frq += snd_SQ1_frq_shadow;

									// disable channel if overflow
									if ((uint)shadow_frq > 2047)
									{
										snd_SQ1_enable = snd_SQ1_swp_enable = false;
									}

									// set negate mode flag that disables channel is negate clerar
									if (snd_SQ1_negate) { snd_SQ1_calc_done = true; }
								}

								if ((snd_SQ1_vol_state == 0) && !snd_SQ1_env_add) { snd_SQ1_enable = snd_SQ1_swp_enable = false; }
							}

							snd_SQ1_len_en = (value & 0x40) > 0;

							value &= 0x40;
							break;

						case 0x66:
						case 0x67:
							// not writable
							value = 0;
							break;

						case 0x68:												// NR21 (sound length / wave pattern duty %)		
							snd_SQ2_duty = (byte)((value & 0xC0) >> 6);
							snd_SQ2_length = (ushort)(64 - (value & 0x3F));
							snd_SQ2_len_cntr = snd_SQ2_length;

							value &= 0xC0;
							break;

						case 0x69:												// NR22 (envelope)
							snd_SQ2_st_vol = (byte)((value & 0xF0) >> 4);
							snd_SQ2_env_add = (value & 8) > 0;
							snd_SQ2_per = (byte)(value & 7);

							// several glitchy effects happen when writing to snd_NRx2 during audio playing
							if (((snd_Audio_Regs[0x09] & 7) == 0) && !snd_SQ2_vol_done) { snd_SQ2_vol_state++; }
							else if ((snd_Audio_Regs[0x09] & 8) == 0) { snd_SQ2_vol_state += 2; }

							if (((snd_Audio_Regs[0x09] ^ value) & 8) > 0) { snd_SQ2_vol_state = (byte)(0x10 - snd_SQ2_vol_state); }

							snd_SQ2_vol_state &= 0xF;
							if ((value & 0xF8) == 0) { snd_SQ2_enable = false; }
							break;

						case 0x6A:
						case 0x6B:
							// not writable
							value = 0;
							break;

						case 0x6C:												// NR23 (freq low)
							snd_SQ2_frq &= 0x700;
							snd_SQ2_frq |= value;

							value = 0;
							break;

						case 0x6D:												// NR24 (freq hi)
							snd_SQ2_trigger = (value & 0x80) > 0;
							snd_SQ2_frq &= 0xFF;
							snd_SQ2_frq |= (ushort)((value & 7) << 8);

							if ((snd_Sequencer_len & 1) == 0)
							{
								if (!snd_SQ2_len_en && ((value & 0x40) > 0) && (snd_SQ2_len_cntr > 0))
								{
									snd_SQ2_len_cntr--;
									if ((snd_SQ2_len_cntr == 0) && !snd_SQ2_trigger) { snd_SQ2_enable = false; }
								}
							}

							if (snd_SQ2_trigger)
							{
								snd_SQ2_enable = true;
								snd_SQ2_vol_done = false;
								snd_SQ2_duty_cntr = 0;

								if (snd_SQ2_len_cntr == 0)
								{
									snd_SQ2_len_cntr = 64;
									if (((value & 0x40) > 0) && ((snd_Sequencer_len & 1) == 0)) { snd_SQ2_len_cntr--; }
								}
								snd_SQ2_intl_cntr = ((2048 - snd_SQ2_frq) * 4) | (snd_SQ2_intl_cntr & 3);
								snd_SQ2_vol_state = snd_SQ2_st_vol;
								snd_SQ2_vol_per = (snd_SQ2_per > 0) ? snd_SQ2_per : 8;
								if (snd_Sequencer_vol == 4) { snd_SQ2_vol_per++; }
								if ((snd_SQ2_vol_state == 0) && !snd_SQ2_env_add) { snd_SQ2_enable = false; }
							}

							snd_SQ2_len_en = (value & 0x40) > 0;

							value &= 0x40;
							break;

						case 0x6E:
						case 0x6F:
							// not writable
							value = 0;
							break;

						case 0x70:												// NR30 (on/off)
							snd_WAVE_DAC_pow = (value & 0x80) > 0;
							if (!snd_WAVE_DAC_pow) { snd_WAVE_enable = false; }

							// selected bank is played, other one can be written to
							snd_Wave_Bank = ((value & 0x40) == 0) ? 16 : 0;

							snd_Wave_Size = (value & 0x20) == 0x20;

							value &= 0xE0;
							break;

						case 0x71:
							value = 0;
							break;

						case 0x72:												// NR31 (length)
							snd_WAVE_length = (ushort)(256 - value);
							snd_WAVE_len_cntr = snd_WAVE_length;

							value =0;
							break;

						case 0x73:												// NR32 (level output)
							snd_WAVE_vol_code = (byte)((value & 0x60) >> 5);
							snd_Wave_Vol_Force = (value & 0x80) == 0x80;

							value &= 0xE0;
							break;

						case 0x74:												// NR33 (freq low)
							snd_WAVE_frq &= 0x700;
							snd_WAVE_frq |= value;

							value = 0;
							break;

						case 0x75:												// NR34 (freq hi)
							snd_WAVE_trigger = (value & 0x80) > 0;
							snd_WAVE_frq &= 0xFF;
							snd_WAVE_frq |= (ushort)((value & 7) << 8);

							if ((snd_Sequencer_len & 1) == 0)
							{

								if (!snd_WAVE_len_en && ((value & 0x40) > 0) && (snd_WAVE_len_cntr > 0))
								{
									snd_WAVE_len_cntr--;
									if ((snd_WAVE_len_cntr == 0) && !snd_WAVE_trigger) { snd_WAVE_enable = false; }
								}
							}

							if (snd_WAVE_trigger)
							{
								snd_WAVE_enable = true;

								if (snd_WAVE_len_cntr == 0)
								{
									snd_WAVE_len_cntr = 256;
									if (((value & 0x40) > 0) && ((snd_Sequencer_len & 1) == 0)) { snd_WAVE_len_cntr--; }
								}
								snd_WAVE_intl_cntr = (2048 - snd_WAVE_frq) * 2 + 6; // trigger delay for wave channel

								snd_WAVE_wave_cntr = 0;
								if (!snd_WAVE_DAC_pow) { snd_WAVE_enable = false; }

								snd_Wave_Bank_Playing = (snd_Wave_Bank == 0) ? 16 : 0;
							}

							snd_WAVE_len_en = (value & 0x40) > 0;

							value &= 0x40;
							break;

						case 0x76:
						case 0x77:
							// not writable
							value = 0;
							break;

						case 0x78:												// NR41 (length)
							snd_NOISE_length = (ushort)(64 - (value & 0x3F));
							snd_NOISE_len_cntr = snd_NOISE_length;

							value = 0;
							break;

						case 0x79:												// NR42 (envelope)
							snd_NOISE_st_vol = (byte)((value & 0xF0) >> 4);
							snd_NOISE_env_add = (value & 8) > 0;
							snd_NOISE_per = (byte)(value & 7);

							// several glitchy effects happen when writing to snd_NRx2 during audio playing
							if (((snd_Audio_Regs[0x19] & 7) == 0) && !snd_NOISE_vol_done) { snd_NOISE_vol_state++; }
							else if ((snd_Audio_Regs[0x19] & 8) == 0) { snd_NOISE_vol_state += 2; }

							if (((snd_Audio_Regs[0x19] ^ value) & 8) > 0) { snd_NOISE_vol_state = (byte)(0x10 - snd_NOISE_vol_state); }

							snd_NOISE_vol_state &= 0xF;
							if ((value & 0xF8) == 0) { snd_NOISE_enable = false; }
							break;

						case 0x7A:
						case 0x7B:
							// not writable
							value = 0;
							break;

						case 0x7C:												// NR43 (shift)
							snd_NOISE_clk_shft = (byte)((value & 0xF0) >> 4);
							snd_NOISE_wdth_md = (value & 8) > 0;
							snd_NOISE_div_code = (byte)(value & 7);
							// Mickey's Dangerous Chase requires writes here to take effect immediately (for sound of taking damage)
							snd_NOISE_intl_cntr = (snd_Divisor[snd_NOISE_div_code] << snd_NOISE_clk_shft);
							break;

						case 0x7D:												// NR44 (trigger)
							snd_NOISE_trigger = (value & 0x80) > 0;

							if ((snd_Sequencer_len & 1) == 0)
							{
								if (!snd_NOISE_len_en && ((value & 0x40) > 0) && (snd_NOISE_len_cntr > 0))
								{
									snd_NOISE_len_cntr--;
									if ((snd_NOISE_len_cntr == 0) && !snd_NOISE_trigger) { snd_NOISE_enable = false; }
								}
							}

							if (snd_NOISE_trigger)
							{
								snd_NOISE_enable = true;
								snd_NOISE_vol_done = false;

								if (snd_NOISE_len_cntr == 0)
								{
									snd_NOISE_len_cntr = 64;
									if (((value & 0x40) > 0) && ((snd_Sequencer_len & 1) == 0)) { snd_NOISE_len_cntr--; }
								}
								snd_NOISE_intl_cntr = (snd_Divisor[snd_NOISE_div_code] << snd_NOISE_clk_shft);
								snd_NOISE_vol_state = snd_NOISE_st_vol;
								snd_NOISE_vol_per = (snd_NOISE_per > 0) ? snd_NOISE_per : 8;
								if (snd_Sequencer_vol == 4) { snd_NOISE_vol_per++; }
								snd_NOISE_LFSR = 0x7FFF;
								if ((snd_NOISE_vol_state == 0) && !snd_NOISE_env_add) { snd_NOISE_enable = false; }
							}

							snd_NOISE_len_en = (value & 0x40) > 0;

							value &= 0x40;
							break;

						case 0x7E:
						case 0x7F:
							// not writable
							value = 0;
							break;

						case 0x80:												// NR50 (ctrl)
							snd_CTRL_vol_L = (byte)((value & 0x70) >> 4);
							snd_CTRL_vol_R = (byte)(value & 7);

							value &= 0x77;
							break;

						case 0x81:												// NR51 (ctrl)
							snd_CTRL_noise_L_en = (value & 0x80) > 0;
							snd_CTRL_wave_L_en = (value & 0x40) > 0;
							snd_CTRL_sq2_L_en = (value & 0x20) > 0;
							snd_CTRL_sq1_L_en = (value & 0x10) > 0;
							snd_CTRL_noise_R_en = (value & 8) > 0;
							snd_CTRL_wave_R_en = (value & 4) > 0;
							snd_CTRL_sq2_R_en = (value & 2) > 0;
							snd_CTRL_sq1_R_en = (value & 1) > 0;
							break;

						case 0x82:                                              // GBA Low (ctrl)
							snd_Chan_Mult = snd_Chan_Mult_Table[value & 3];

							snd_FIFO_A_Mult = ((value & 0x04) == 0x04) ? 4 : 2;
							snd_FIFO_B_Mult = ((value & 0x08) == 0x08) ? 4 : 2;

							value &= 0xF;
							break;

						case 0x83:                                              // GBA High (ctrl)
							snd_FIFO_A_Enable_R = (value & 1) == 1;
							snd_FIFO_A_Enable_L = (value & 2) == 2;

							snd_FIFO_B_Enable_R = (value & 0x10) == 0x10;
							snd_FIFO_B_Enable_L = (value & 0x20) == 0x20;

							snd_FIFO_A_Timer = (value & 0x04) >> 2;
							snd_FIFO_B_Timer = (value & 0x40) >> 6;

							if ((value & 0x08) == 0x08)
							{
								for (int i = 0; i < 32; i++)
								{
									snd_FIFO_A[i] = 0;
								}

								snd_FIFO_A_ptr = 0;
							}

							if ((value & 0x80) == 0x80)
							{
								for (int i = 0; i < 32; i++)
								{
									snd_FIFO_B[i] = 0;
								}

								snd_FIFO_B_ptr = 0;
							}
							value &= 0x77;						
							break;

						case 0x84:												// NR52 (ctrl)						
							// NOTE: Make sure to do the power off first since it will call the write_reg function again
							if ((value & 0x80) == 0) { power_off(); }
							snd_CTRL_power = (value & 0x80) > 0;

							value &= 0x80;
							break;

						case 0x85:
							// not writable
							value = 0;
							break;

						case 0x86:
						case 0x87:
							// not writable
							value = 0;
							break;

						case 0x88:                                              // Bias Control
							snd_Bias_Low = value;

							snd_BIAS_Offset &= unchecked((int)0xFFFFFF00);
							snd_BIAS_Offset |= snd_Bias_Low;
							break;

						case 0x89:
							snd_Bias_High = value;

							snd_BIAS_Offset &= unchecked((int)0x000000FF);
							snd_BIAS_Offset |= ((snd_Bias_High & 0x3) << 8);

							if ((snd_BIAS_Offset & 0x200) == 0x200)
							{
								snd_BIAS_Offset |= unchecked((int)0xFFFFFC00);
							}

							switch (value & 0xC0)
							{
								case 0x00: snd_Sample_Rate = 0x1FF; break;
								case 0x40: snd_Sample_Rate = 0xFF; break;
								case 0x80: snd_Sample_Rate = 0x7F; break;
								case 0xC0: snd_Sample_Rate = 0x3F; break;
							}

							break;

						case 0x8A:
						case 0x8B:
							// not writable
							value = 0;
							break;
					}

					snd_Audio_Regs[addr - 0x60] = value;
					snd_Update_NR52();
				}
				else
				{

				}
			}
			// when power is off, only length counters and waveRAM are effected by writes
			// FIFO regs are also enabled
			else
			{
				switch (addr)
				{
					case 0x82:                                              // GBA Low (ctrl)
						snd_Chan_Mult = snd_Chan_Mult_Table[value & 3];

						snd_FIFO_A_Mult = ((value & 0x04) == 0x04) ? 4 : 2;
						snd_FIFO_B_Mult = ((value & 0x08) == 0x08) ? 4 : 2;

						value &= 0xF;
						break;

					case 0x83:                                              // GBA High (ctrl)
						snd_FIFO_A_Enable_R = (value & 1) == 1;
						snd_FIFO_A_Enable_L = (value & 2) == 2;

						snd_FIFO_B_Enable_R = (value & 0x10) == 0x10;
						snd_FIFO_B_Enable_L = (value & 0x20) == 0x20;

						snd_FIFO_A_Timer = (value & 0x04) >> 2;
						snd_FIFO_B_Timer = (value & 0x40) >> 6;

						if ((value & 0x08) == 0x08)
						{
							for (int i = 0; i < 32; i++)
							{
								snd_FIFO_A[i] = 0;
							}

							snd_FIFO_A_ptr = 0;
						}

						if ((value & 0x80) == 0x80)
						{
							for (int i = 0; i < 32; i++)
							{
								snd_FIFO_B[i] = 0;
							}

							snd_FIFO_B_ptr = 0;
						}

						value &= 0x77;
						break;

					case 0x84:                                        // NR52 (ctrl)
						snd_CTRL_power = (value & 0x80) > 0;
						if (snd_CTRL_power)
						{
							snd_Sequencer_reset_cd = 4;
							Console.WriteLine("Aud Power on");
						}

						value &= 0x80;
						snd_Audio_Regs[addr - 0x60] = value;
						snd_Update_NR52();
						break;
					case 0x88:                                        // Bias Control
						snd_Bias_Low = value;

						snd_BIAS_Offset &= unchecked((int)0xFFFFFF00);
						snd_BIAS_Offset |= snd_Bias_Low;

						snd_Audio_Regs[addr - 0x60] = value;
						break;
					case 0x89:                                        // Bias Control
						snd_Bias_High = value;

						snd_BIAS_Offset &= unchecked((int)0x000000FF);
						snd_BIAS_Offset |= ((snd_Bias_High & 0x3) << 8);

						if ((snd_BIAS_Offset & 0x200) == 0x200)
						{
							snd_BIAS_Offset |= unchecked((int)0xFFFFFC00);
						}

						switch (value & 0xC0)
						{
							case 0x00: snd_Sample_Rate = 0x1FF; break;
							case 0x40: snd_Sample_Rate = 0xFF; break;
							case 0x80: snd_Sample_Rate = 0x7F; break;
							case 0xC0: snd_Sample_Rate = 0x3F; break;
						}

						snd_Audio_Regs[addr - 0x60] = value;
						break;

					default:
						value &= 0;
						break;
				}

				snd_Audio_Regs[addr - 0x60] = value;
			}
		}

		public void snd_Tick()
		{
			snd_Divider++;
			if ((snd_Divider & 3) == 0)
			{
				// calculate square1's output
				if (snd_SQ1_enable)
				{
					snd_SQ1_intl_cntr--;
					if (snd_SQ1_intl_cntr == 0)
					{
						snd_SQ1_intl_cntr = (2048 - snd_SQ1_frq) * 4;
						snd_SQ1_duty_cntr++;
						snd_SQ1_duty_cntr &= 7;

						if (snd_Duty_Cycles[snd_SQ1_duty * 8 + snd_SQ1_duty_cntr])
						{
							snd_SQ1_output = snd_SQ1_vol_state * 2;
						}
						else
						{
							snd_SQ1_output = 0;
						}
					}
				}

				// calculate square2's output
				if (snd_SQ2_enable)
				{
					snd_SQ2_intl_cntr--;
					if (snd_SQ2_intl_cntr == 0)
					{
						snd_SQ2_intl_cntr = (2048 - snd_SQ2_frq) * 4;
						snd_SQ2_duty_cntr++;
						snd_SQ2_duty_cntr &= 7;

						if (snd_Duty_Cycles[snd_SQ2_duty * 8 + snd_SQ2_duty_cntr])
						{
							snd_SQ2_output = snd_SQ2_vol_state * 2;
						}
						else
						{
							snd_SQ2_output =  0;
						}
					}
				}

				// calculate wave output
				snd_WAVE_can_get = false;
				if (snd_WAVE_enable)
				{
					snd_WAVE_intl_cntr--;

					if (snd_WAVE_intl_cntr == 0)
					{
						snd_WAVE_can_get = true;

						snd_WAVE_intl_cntr = (2048 - snd_WAVE_frq) * 2;

						snd_Sample = (byte)((snd_Wave_RAM[snd_Wave_Bank_Playing] >> 4) & 0xF);

						if (snd_Wave_Vol_Force)
						{
							snd_Sample = (byte)(((snd_Sample * 3) >> 2) & 0xF);
						}
						else
						{
							if (snd_WAVE_vol_code == 0)
							{
								snd_Sample = (byte)((snd_Sample & 0xF) >> 4);
							}
							else if (snd_WAVE_vol_code == 1)
							{
								snd_Sample = (byte)(snd_Sample & 0xF);
							}
							else if (snd_WAVE_vol_code == 2)
							{
								snd_Sample = (byte)((snd_Sample & 0xF) >> 1);
							}
							else
							{
								snd_Sample = (byte)((snd_Sample & 0xF) >> 2);
							}
						}
					
						snd_WAVE_output = snd_Sample * 2;

						byte temp_samp = (byte)((snd_Wave_RAM[snd_Wave_Bank_Playing] & 0xF0) >> 4);

						snd_Wave_RAM[snd_Wave_Bank_Playing] = (byte)((snd_Wave_RAM[snd_Wave_Bank_Playing] & 0xF) << 4);

						for (int i = 1; i <= 15; i++)
						{
							snd_Wave_RAM[snd_Wave_Bank_Playing + i - 1] |= (byte)((snd_Wave_RAM[snd_Wave_Bank_Playing + i] & 0xF0) >> 4);

							snd_Wave_RAM[snd_Wave_Bank_Playing + i] = (byte)((snd_Wave_RAM[snd_Wave_Bank_Playing + i] & 0xF) << 4);
						}

						snd_Wave_RAM[snd_Wave_Bank_Playing + 15] |= temp_samp;

						// NOTE: The snd_Sample buffer is only reloaded after the current snd_Sample is played, even if just triggered
						snd_WAVE_wave_cntr++;

						// swap playing banks if selected
						if (snd_Wave_Size)
						{
							if (snd_WAVE_wave_cntr == 32)
							{
								if (snd_Wave_Bank_Playing == 16)
								{
									snd_Wave_Bank_Playing = 0;
								}
								else 
								{
									snd_Wave_Bank_Playing = 16;
								}
							}
						}

						snd_WAVE_wave_cntr &= 0x1F;
					}
				}

				// calculate noise output
				if (snd_NOISE_enable)
				{
					snd_NOISE_intl_cntr--;
					if (snd_NOISE_intl_cntr == 0)
					{
						snd_NOISE_intl_cntr = (snd_Divisor[snd_NOISE_div_code] << snd_NOISE_clk_shft);
						int bit_lfsr = (snd_NOISE_LFSR & 1) ^ ((snd_NOISE_LFSR & 2) >> 1);

						snd_NOISE_LFSR = (snd_NOISE_LFSR >> 1) & 0x3FFF;
						snd_NOISE_LFSR |= (bit_lfsr << 14);

						if (snd_NOISE_wdth_md)
						{
							snd_NOISE_LFSR = snd_NOISE_LFSR & 0x7FBF;
							snd_NOISE_LFSR |= (bit_lfsr << 6);
						}


						if ((snd_NOISE_LFSR & 1) > 0)
						{
							snd_NOISE_output = 0;
						}
						else
						{
							snd_NOISE_output = snd_NOISE_vol_state * 2;
						}
					}
				}

				// Frame Sequencer ticks at a rate of 512 hz
				snd_Internal_cnt++;
				snd_Internal_cnt &= 0x1FFF;

				if ((snd_Internal_cnt == 0) && snd_CTRL_power)
				{				
					snd_Sequencer_vol++; snd_Sequencer_vol &= 0x7;
					snd_Sequencer_len++; snd_Sequencer_len &= 0x7;
					snd_Sequencer_swp++; snd_Sequencer_swp &= 0x7;

					// clock the lengths
					if ((snd_Sequencer_len & 1) == 0)
					{
						if (snd_SQ1_len_en && snd_SQ1_len_cntr > 0)
						{
							snd_SQ1_len_cntr--;
							if (snd_SQ1_len_cntr == 0) { snd_SQ1_enable = snd_SQ1_swp_enable = false; }
						}
						if (snd_SQ2_len_en && snd_SQ2_len_cntr > 0)
						{
							snd_SQ2_len_cntr--;
							if (snd_SQ2_len_cntr == 0) { snd_SQ2_enable = false; }
						}
						if (snd_WAVE_len_en && snd_WAVE_len_cntr > 0)
						{
							snd_WAVE_len_cntr--;
							if (snd_WAVE_len_cntr == 0) { snd_WAVE_enable = false; }
						}
						if (snd_NOISE_len_en && snd_NOISE_len_cntr > 0)
						{
							snd_NOISE_len_cntr--;
							if (snd_NOISE_len_cntr == 0) { snd_NOISE_enable = false; }
						}

						snd_Update_NR52();
					}

					// clock the sweep
					if ((snd_Sequencer_swp == 0) || (snd_Sequencer_swp == 4))
					{
						snd_SQ1_intl_swp_cnt--;
						if ((snd_SQ1_intl_swp_cnt == 0) && snd_SQ1_swp_enable)
						{
							snd_SQ1_intl_swp_cnt = snd_SQ1_swp_prd > 0 ? snd_SQ1_swp_prd : 8;

							if ((snd_SQ1_swp_prd > 0))
							{
								int shadow_frq = snd_SQ1_frq_shadow;
								shadow_frq = shadow_frq >> snd_SQ1_shift;
								if (snd_SQ1_negate) { shadow_frq = -shadow_frq; }
								shadow_frq += snd_SQ1_frq_shadow;

								// set negate mode flag that disables channel is negate clerar
								if (snd_SQ1_negate) { snd_SQ1_calc_done = true; }

								// disable channel if overflow
								if ((uint)shadow_frq > 2047)
								{
									snd_SQ1_enable = snd_SQ1_swp_enable = false;
								}
								else
								{
									if (snd_SQ1_shift > 0)
									{
										// NOTE: no need to write back to registers as they are not readable
										shadow_frq &= 0x7FF;
										snd_SQ1_frq = shadow_frq;
										snd_SQ1_frq_shadow = shadow_frq;

										// after writing, we repeat the process and do another overflow check
										shadow_frq = snd_SQ1_frq_shadow;
										shadow_frq = shadow_frq >> snd_SQ1_shift;
										if (snd_SQ1_negate) { shadow_frq = -shadow_frq; }
										shadow_frq += snd_SQ1_frq_shadow;

										if ((uint)shadow_frq > 2047)
										{
											snd_SQ1_enable = snd_SQ1_swp_enable = false;
										}
									}
								}
							}

							snd_Update_NR52();
						}
					}

					// clock the volume envelope
					if (snd_Sequencer_vol == 5)
					{
						if (snd_SQ1_per > 0)
						{
							snd_SQ1_vol_per--;
							if (snd_SQ1_vol_per == 0)
							{
								snd_SQ1_vol_per = (snd_SQ1_per > 0) ? snd_SQ1_per : 8;
								if (!snd_SQ1_vol_done)
								{
									if (snd_SQ1_env_add)
									{
										if (snd_SQ1_vol_state < 15) { snd_SQ1_vol_state++; }
										else { snd_SQ1_vol_done = true; }
									}
									else
									{
										if (snd_SQ1_vol_state >= 1) { snd_SQ1_vol_state--; }
										else { snd_SQ1_vol_done = true; }
									}
								}
							}
						}

						if (snd_SQ2_per > 0)
						{
							snd_SQ2_vol_per--;
							if (snd_SQ2_vol_per == 0)
							{
								snd_SQ2_vol_per = (snd_SQ2_per > 0) ? snd_SQ2_per : 8;
								if (!snd_SQ2_vol_done)
								{
									if (snd_SQ2_env_add)
									{
										if (snd_SQ2_vol_state < 15) { snd_SQ2_vol_state++; }
										else { snd_SQ2_vol_done = true; }
									}
									else
									{
										if (snd_SQ2_vol_state >= 1) { snd_SQ2_vol_state--; }
										else { snd_SQ2_vol_done = true; }
									}
								}
							}
						}

						if (snd_NOISE_per > 0)
						{
							snd_NOISE_vol_per--;
							if (snd_NOISE_vol_per == 0)
							{
								snd_NOISE_vol_per = (snd_NOISE_per > 0) ? snd_NOISE_per : 8;
								if (!snd_NOISE_vol_done)
								{
									if (snd_NOISE_env_add)
									{
										if (snd_NOISE_vol_state < 15) { snd_NOISE_vol_state++; }
										else { snd_NOISE_vol_done = true; }
									}
									else
									{
										if (snd_NOISE_vol_state >= 1) { snd_NOISE_vol_state--; }
										else { snd_NOISE_vol_done = true; }
									}
								}
							}
						}
					}
				}

				if (snd_Sequencer_reset_cd > 0)
				{
					snd_Sequencer_reset_cd--;
							
					if (snd_Sequencer_reset_cd == 0)
					{
						// any side effects like GBC?
						snd_Sequencer_len = 0;
						snd_Sequencer_vol = 0;
						snd_Sequencer_swp = 0;
					}
				}
			}

			// caclulate FIFO outputs
			if (snd_FIFO_A_Tick)
			{
				if (snd_FIFO_A_ptr > 0)
				{
					snd_FIFO_A_Sample = snd_FIFO_A[0];

					for (int i = 1; i <= 31; i++)
					{
						snd_FIFO_A[i - 1] = snd_FIFO_A[i];
					}

					snd_FIFO_A[31] = 0;

					snd_FIFO_A_ptr -= 1;				
				}

				if (snd_FIFO_A_ptr <= 14)
				{
					if (dma_Go[1] && dma_Start_Snd_Vid[1])
					{
						FIFO_DMA_A_cd = 3;
						FIFO_DMA_A_Delay = true;
						Misc_Delays = true;
						delays_to_process = true;					
					}
				}

				snd_FIFO_A_Output = ((sbyte)(snd_FIFO_A_Sample)) * snd_FIFO_A_Mult;
				//Console.WriteLine("A: " + snd_FIFO_A_Sample + " " + snd_FIFO_A_Output);

				snd_FIFO_A_Tick = false;
			}

			if (snd_FIFO_B_Tick)
			{
				if (snd_FIFO_B_ptr > 0)
				{
					snd_FIFO_B_Sample = snd_FIFO_B[0];

					for (int i = 1; i <= 31; i++)
					{
						snd_FIFO_B[i - 1] = snd_FIFO_B[i];
					}

					snd_FIFO_B[31] = 0;

					snd_FIFO_B_ptr -= 1;		
				}

				if (snd_FIFO_B_ptr <= 14)
				{
					if (dma_Go[2] && dma_Start_Snd_Vid[2])
					{
						FIFO_DMA_B_cd = 3;
						FIFO_DMA_B_Delay = true;
						Misc_Delays = true;
						delays_to_process = true;
					}
				}

				snd_FIFO_B_Output = ((sbyte)(snd_FIFO_B_Sample)) * snd_FIFO_B_Mult;
				//Console.WriteLine("B: " + snd_FIFO_B_Sample + " " + snd_FIFO_B_Output);

				snd_FIFO_B_Tick = false;
			}

			if ((snd_Divider & snd_Sample_Rate) == 0)
			{
				// add up components to each channel
				int L_final = 0;
				int R_final = 0;

				if (snd_CTRL_sq1_L_en) { L_final += snd_SQ1_output; }
				if (snd_CTRL_sq2_L_en) { L_final += snd_SQ2_output; }
				if (snd_CTRL_wave_L_en) { L_final += snd_WAVE_output; }
				if (snd_CTRL_noise_L_en) { L_final += snd_NOISE_output; }

				// channels 1-4 are effected by L/R volume controls, but not FIFO
				L_final *= (snd_CTRL_vol_L + 1);

				if (snd_FIFO_A_Enable_L) { L_final += snd_FIFO_A_Output; }
				if (snd_FIFO_B_Enable_L) { L_final += snd_FIFO_B_Output; }

				if (snd_CTRL_sq1_R_en) { R_final += snd_SQ1_output; }
				if (snd_CTRL_sq2_R_en) { R_final += snd_SQ2_output; }
				if (snd_CTRL_wave_R_en) { R_final += snd_WAVE_output; }
				if (snd_CTRL_noise_R_en) { R_final += snd_NOISE_output; }

				// channels 1-4 are effected by L/R volume controls, but not FIFO
				R_final *= (snd_CTRL_vol_R + 1);

				if (snd_FIFO_A_Enable_R) { R_final += snd_FIFO_A_Output; }
				if (snd_FIFO_B_Enable_R) { R_final += snd_FIFO_B_Output; }

				//Console.WriteLine("F " + snd_FIFO_A_Output + " " + snd_FIFO_B_Output + " " + (snd_FIFO_A_Output + snd_FIFO_B_Output));

				L_final += snd_BIAS_Offset;
				R_final += snd_BIAS_Offset;

				//L_final &= 0x3FF;
				//R_final &= 0x3FF;

				L_final *= 24;
				R_final *= 24;

				if (L_final != snd_Latched_Sample_L)
				{
					_blip_L.AddDelta(snd_Master_Clock, L_final - snd_Latched_Sample_L);
					snd_Latched_Sample_L = L_final;
				}

				if (R_final != snd_Latched_Sample_R)
				{
					_blip_R.AddDelta(snd_Master_Clock, R_final - snd_Latched_Sample_R);
					snd_Latched_Sample_R = R_final;
				}
			}

			snd_Master_Clock++;
		}

		public void power_off()
		{
			for (uint i = 0x60; i < 0x82; i++)
			{
				snd_Write_Reg_8(i, 0);
			}

			for (int i = 0; i < 32; i++)
			{
				snd_FIFO_A[i] = 0;
				snd_FIFO_B[i] = 0;
			}

			// duty and length are reset
			snd_SQ1_duty_cntr = snd_SQ2_duty_cntr = 0;

			snd_FIFO_A_ptr = snd_FIFO_B_ptr = 0;

			// reset state variables
			snd_SQ1_enable = snd_SQ1_swp_enable = snd_SQ2_enable = snd_WAVE_enable = snd_NOISE_enable = false;

			snd_SQ1_len_en = snd_SQ2_len_en = snd_WAVE_len_en = snd_NOISE_len_en = false;

			snd_SQ1_output = snd_SQ2_output = snd_WAVE_output = snd_NOISE_output = 0;

			// on GBC, lengths are also reset
			snd_SQ1_length = snd_SQ2_length = snd_WAVE_length = snd_NOISE_length = 0;
			snd_SQ1_len_cntr = snd_SQ2_len_cntr = snd_WAVE_len_cntr = snd_NOISE_len_cntr = 0;

			snd_FIFO_A_Sample = snd_FIFO_B_Sample = 0;

			snd_FIFO_A_Output = snd_FIFO_B_Output = 0;

			snd_Update_NR52();
		}

		public void snd_Reset()
		{
			for (int i = 0; i < 32; i++)
			{
				snd_Wave_RAM[i] = (byte)(((i & 1) == 0) ? 0 : 0xFF);
				snd_FIFO_A[i] = 0;
				snd_FIFO_B[i] = 0;
			}

			for (int i = 0; i < 0x30; i++)
			{
				snd_Audio_Regs[i] = 0;
			}

			for (int i = 0; i < 0x30; i++)
			{
				snd_Write_Reg_8((uint)(0x60 + i), 0);
			}

			snd_SQ1_duty_cntr = snd_SQ2_duty_cntr = 0;

			snd_SQ1_enable = snd_SQ1_swp_enable = snd_SQ2_enable = snd_WAVE_enable = snd_NOISE_enable = false;

			snd_SQ1_len_en = snd_SQ2_len_en = snd_WAVE_len_en = snd_NOISE_len_en = false;

			snd_SQ1_output = snd_SQ2_output = snd_WAVE_output = snd_NOISE_output = 0;

			snd_SQ1_length = snd_SQ2_length = snd_WAVE_length = snd_NOISE_length = 0;

			snd_SQ1_len_cntr = snd_SQ2_len_cntr = snd_WAVE_len_cntr = snd_NOISE_len_cntr = 0;

			snd_Master_Clock = 0;

			snd_Internal_cnt = snd_Divider = 0;

			snd_Sequencer_len = 0;
			snd_Sequencer_swp = 0;
			snd_Sequencer_vol = 0;
			snd_Sequencer_reset_cd = 0;

			snd_Sample = 0;

			snd_CTRL_GBA_Low = snd_CTRL_GBA_High = 0;
			snd_Bias_Low = snd_Bias_High = 0;

			snd_Wave_Bank = snd_Wave_Bank_Playing = 0;

			snd_FIFO_A_ptr = snd_FIFO_B_ptr = 0;

			snd_FIFO_A_Sample = snd_FIFO_B_Sample = 0;

			snd_FIFO_A_Output = snd_FIFO_B_Output = 0;

			snd_BIAS_Offset = 0;

			snd_Sample_Rate = 0x1FF;

			snd_Update_NR52();

			_blip_L.SetRates(4194304 * 4, 44100);
			_blip_R.SetRates(4194304 * 4, 44100);
		}

		public void snd_SyncState(Serializer ser)
		{
			ser.Sync(nameof(snd_Audio_Regs), ref snd_Audio_Regs, false);
			ser.Sync(nameof(snd_Wave_RAM), ref snd_Wave_RAM, false);
	
			ser.Sync(nameof(snd_SQ1_vol_done), ref snd_SQ1_vol_done);
			ser.Sync(nameof(snd_SQ1_calc_done), ref snd_SQ1_calc_done);
			ser.Sync(nameof(snd_SQ1_swp_enable), ref snd_SQ1_swp_enable);
			ser.Sync(nameof(snd_SQ1_enable), ref snd_SQ1_enable);
			ser.Sync(nameof(snd_SQ1_vol_state), ref snd_SQ1_vol_state);
			ser.Sync(nameof(snd_SQ1_duty_cntr), ref snd_SQ1_duty_cntr);
			ser.Sync(nameof(snd_SQ1_frq_shadow), ref snd_SQ1_frq_shadow);
			ser.Sync(nameof(snd_SQ1_intl_cntr), ref snd_SQ1_intl_cntr);
			ser.Sync(nameof(snd_SQ1_vol_per), ref snd_SQ1_vol_per);
			ser.Sync(nameof(snd_SQ1_intl_swp_cnt), ref snd_SQ1_intl_swp_cnt);
			ser.Sync(nameof(snd_SQ1_len_cntr), ref snd_SQ1_len_cntr);
			ser.Sync(nameof(snd_SQ1_negate), ref snd_SQ1_negate);
			ser.Sync(nameof(snd_SQ1_trigger), ref snd_SQ1_trigger);
			ser.Sync(nameof(snd_SQ1_len_en), ref snd_SQ1_len_en);
			ser.Sync(nameof(snd_SQ1_env_add), ref snd_SQ1_env_add);
			ser.Sync(nameof(snd_SQ1_shift), ref snd_SQ1_shift);
			ser.Sync(nameof(snd_SQ1_duty), ref snd_SQ1_duty);
			ser.Sync(nameof(snd_SQ1_st_vol), ref snd_SQ1_st_vol);
			ser.Sync(nameof(snd_SQ1_per), ref snd_SQ1_per);
			ser.Sync(nameof(snd_SQ1_swp_prd), ref snd_SQ1_swp_prd);
			ser.Sync(nameof(snd_SQ1_frq), ref snd_SQ1_frq);
			ser.Sync(nameof(snd_SQ1_length), ref snd_SQ1_length);
			ser.Sync(nameof(snd_SQ1_output), ref snd_SQ1_output);

			ser.Sync(nameof(snd_SQ2_vol_done), ref snd_SQ2_vol_done);
			ser.Sync(nameof(snd_SQ2_enable), ref snd_SQ2_enable);
			ser.Sync(nameof(snd_SQ2_vol_state), ref snd_SQ2_vol_state);
			ser.Sync(nameof(snd_SQ2_duty_cntr), ref snd_SQ2_duty_cntr);
			ser.Sync(nameof(snd_SQ2_intl_cntr), ref snd_SQ2_intl_cntr);
			ser.Sync(nameof(snd_SQ2_vol_per), ref snd_SQ2_vol_per);
			ser.Sync(nameof(snd_SQ2_len_cntr), ref snd_SQ2_len_cntr);
			ser.Sync(nameof(snd_SQ2_trigger), ref snd_SQ2_trigger);
			ser.Sync(nameof(snd_SQ2_len_en), ref snd_SQ2_len_en);
			ser.Sync(nameof(snd_SQ2_env_add), ref snd_SQ2_env_add);
			ser.Sync(nameof(snd_SQ2_duty), ref snd_SQ2_duty);
			ser.Sync(nameof(snd_SQ2_st_vol), ref snd_SQ2_st_vol);
			ser.Sync(nameof(snd_SQ2_per), ref snd_SQ2_per);
			ser.Sync(nameof(snd_SQ2_frq), ref snd_SQ2_frq);
			ser.Sync(nameof(snd_SQ2_length), ref snd_SQ2_length);
			ser.Sync(nameof(snd_SQ2_output), ref snd_SQ2_output);

			ser.Sync(nameof(snd_WAVE_can_get), ref snd_WAVE_can_get);
			ser.Sync(nameof(snd_WAVE_enable), ref snd_WAVE_enable);
			ser.Sync(nameof(snd_WAVE_wave_cntr), ref snd_WAVE_wave_cntr);
			ser.Sync(nameof(snd_WAVE_intl_cntr), ref snd_WAVE_intl_cntr);
			ser.Sync(nameof(snd_WAVE_len_cntr), ref snd_WAVE_len_cntr);
			ser.Sync(nameof(snd_WAVE_DAC_pow), ref snd_WAVE_DAC_pow);
			ser.Sync(nameof(snd_WAVE_trigger), ref snd_WAVE_trigger);
			ser.Sync(nameof(snd_WAVE_len_en), ref snd_WAVE_len_en);
			ser.Sync(nameof(snd_WAVE_vol_code), ref snd_WAVE_vol_code);
			ser.Sync(nameof(snd_WAVE_frq), ref snd_WAVE_frq);
			ser.Sync(nameof(snd_WAVE_length), ref snd_WAVE_length);
			ser.Sync(nameof(snd_WAVE_output), ref snd_WAVE_output);

			ser.Sync(nameof(snd_NOISE_vol_done), ref snd_NOISE_vol_done);
			ser.Sync(nameof(snd_NOISE_enable), ref snd_NOISE_enable);
			ser.Sync(nameof(snd_NOISE_vol_state), ref snd_NOISE_vol_state);
			ser.Sync(nameof(snd_NOISE_intl_cntr), ref snd_NOISE_intl_cntr);
			ser.Sync(nameof(snd_NOISE_vol_per), ref snd_NOISE_vol_per);
			ser.Sync(nameof(snd_NOISE_LFSR), ref snd_NOISE_LFSR);
			ser.Sync(nameof(snd_NOISE_len_cntr), ref snd_NOISE_len_cntr);
			ser.Sync(nameof(snd_NOISE_wdth_md), ref snd_NOISE_wdth_md);
			ser.Sync(nameof(snd_NOISE_trigger), ref snd_NOISE_trigger);
			ser.Sync(nameof(snd_NOISE_len_en), ref snd_NOISE_len_en);
			ser.Sync(nameof(snd_NOISE_env_add), ref snd_NOISE_env_add);
			ser.Sync(nameof(snd_NOISE_clk_shft), ref snd_NOISE_clk_shft);
			ser.Sync(nameof(snd_NOISE_div_code), ref snd_NOISE_div_code);
			ser.Sync(nameof(snd_NOISE_st_vol), ref snd_NOISE_st_vol);
			ser.Sync(nameof(snd_NOISE_per), ref snd_NOISE_per);
			ser.Sync(nameof(snd_NOISE_length), ref snd_NOISE_length);
			ser.Sync(nameof(snd_NOISE_output), ref snd_NOISE_output);

			ser.Sync(nameof(snd_Sequencer_len), ref snd_Sequencer_len);
			ser.Sync(nameof(snd_Sequencer_vol), ref snd_Sequencer_vol);
			ser.Sync(nameof(snd_Sequencer_swp), ref snd_Sequencer_swp);
			ser.Sync(nameof(snd_Sequencer_reset_cd), ref snd_Sequencer_reset_cd);
			ser.Sync(nameof(snd_Wave_Decay_cnt), ref snd_Wave_Decay_cnt);
			ser.Sync(nameof(snd_Internal_cnt), ref snd_Internal_cnt);
			ser.Sync(nameof(snd_Divider), ref snd_Divider);
			ser.Sync(nameof(snd_Wave_Decay_Done), ref snd_Wave_Decay_Done);

			ser.Sync(nameof(snd_Sample), ref snd_Sample);
			ser.Sync(nameof(snd_Latched_Sample_L), ref snd_Latched_Sample_L);
			ser.Sync(nameof(snd_Latched_Sample_R), ref snd_Latched_Sample_R);
			ser.Sync(nameof(snd_BIAS_Offset), ref snd_BIAS_Offset);
			ser.Sync(nameof(snd_Sample_Rate), ref snd_Sample_Rate);

			ser.Sync(nameof(snd_CTRL_sq1_L_en), ref snd_CTRL_sq1_L_en);
			ser.Sync(nameof(snd_CTRL_sq2_L_en), ref snd_CTRL_sq2_L_en);
			ser.Sync(nameof(snd_CTRL_wave_L_en), ref snd_CTRL_wave_L_en);
			ser.Sync(nameof(snd_CTRL_noise_L_en), ref snd_CTRL_noise_L_en);
			ser.Sync(nameof(snd_CTRL_sq1_R_en), ref snd_CTRL_sq1_R_en);
			ser.Sync(nameof(snd_CTRL_sq2_R_en), ref snd_CTRL_sq2_R_en);
			ser.Sync(nameof(snd_CTRL_wave_R_en), ref snd_CTRL_wave_R_en);
			ser.Sync(nameof(snd_CTRL_noise_R_en), ref snd_CTRL_noise_R_en);
			ser.Sync(nameof(snd_CTRL_power), ref snd_CTRL_power);
			ser.Sync(nameof(snd_CTRL_vol_L), ref snd_CTRL_vol_L);
			ser.Sync(nameof(snd_CTRL_vol_R), ref snd_CTRL_vol_R);

			ser.Sync(nameof(snd_CTRL_GBA_Low), ref snd_CTRL_GBA_Low);
			ser.Sync(nameof(snd_CTRL_GBA_High), ref snd_CTRL_GBA_High);
			ser.Sync(nameof(snd_Wave_Bank), ref snd_Wave_Bank);
			ser.Sync(nameof(snd_Wave_Bank_Playing), ref snd_Wave_Bank_Playing);
			ser.Sync(nameof(snd_Chan_Mult), ref snd_Chan_Mult);
			ser.Sync(nameof(snd_Bias_Low), ref snd_Bias_Low);
			ser.Sync(nameof(snd_Bias_High), ref snd_Bias_High);
			ser.Sync(nameof(snd_Wave_Size), ref snd_Wave_Size);
			ser.Sync(nameof(snd_Wave_Vol_Force), ref snd_Wave_Vol_Force);

			ser.Sync(nameof(snd_FIFO_A_ptr), ref snd_FIFO_A_ptr);
			ser.Sync(nameof(snd_FIFO_B_ptr), ref snd_FIFO_B_ptr);
			ser.Sync(nameof(snd_FIFO_A_Timer), ref snd_FIFO_A_Timer);
			ser.Sync(nameof(snd_FIFO_B_Timer), ref snd_FIFO_B_Timer);
			ser.Sync(nameof(snd_FIFO_A_Output), ref snd_FIFO_A_Output);
			ser.Sync(nameof(snd_FIFO_B_Output), ref snd_FIFO_B_Output);
			ser.Sync(nameof(snd_FIFO_A_Mult), ref snd_FIFO_A_Mult);
			ser.Sync(nameof(snd_FIFO_B_Mult), ref snd_FIFO_B_Mult);
			ser.Sync(nameof(snd_FIFO_A), ref snd_FIFO_A, false);
			ser.Sync(nameof(snd_FIFO_B), ref snd_FIFO_B, false);
			ser.Sync(nameof(snd_FIFO_A_Data), ref snd_FIFO_A_Data, false);
			ser.Sync(nameof(snd_FIFO_B_Data), ref snd_FIFO_B_Data, false);
			ser.Sync(nameof(snd_FIFO_A_Sample), ref snd_FIFO_A_Sample);
			ser.Sync(nameof(snd_FIFO_B_Sample), ref snd_FIFO_B_Sample);
			ser.Sync(nameof(snd_FIFO_A_Tick), ref snd_FIFO_A_Tick);
			ser.Sync(nameof(snd_FIFO_B_Tick), ref snd_FIFO_B_Tick);
			ser.Sync(nameof(snd_FIFO_A_Enable_L), ref snd_FIFO_A_Enable_L);
			ser.Sync(nameof(snd_FIFO_B_Enable_L), ref snd_FIFO_B_Enable_L);
			ser.Sync(nameof(snd_FIFO_A_Enable_R), ref snd_FIFO_A_Enable_R);
			ser.Sync(nameof(snd_FIFO_B_Enable_R), ref snd_FIFO_B_Enable_R);
		}

		public void snd_Update_NR52()
		{
			snd_Audio_Regs[0x24] = (byte)(
								   ((snd_CTRL_power ? 1 : 0) << 7) |
								   (snd_SQ1_enable ? 1 : 0) |
								   ((snd_SQ2_enable ? 1 : 0) << 1) |
								   ((snd_WAVE_enable ? 1 : 0) << 2) |
								   ((snd_NOISE_enable ? 1 : 0) << 3));
		}

		// sound provider code

		private BlipBuffer _blip_L = new BlipBuffer(15000);
		private BlipBuffer _blip_R = new BlipBuffer(15000);

		public bool CanProvideAsync => false;

		public void SetSyncMode(SyncSoundMode mode)
		{
			if (mode != SyncSoundMode.Sync)
			{
				throw new InvalidOperationException("Only Sync mode is supported_");
			}
		}

		public SyncSoundMode SyncMode => SyncSoundMode.Sync;

		public void GetSamplesSync(out short[] snd_Samples, out int nsamp)
		{
			_blip_L.EndFrame(snd_Master_Clock);
			_blip_R.EndFrame(snd_Master_Clock);

			nsamp = _blip_L.SamplesAvailable();

			snd_Samples = new short[nsamp * 2];

			if (nsamp != 0)
			{
				_blip_L.ReadSamplesLeft(snd_Samples, nsamp);
				_blip_R.ReadSamplesRight(snd_Samples, nsamp);
			}

			snd_Master_Clock = 0;
		}

		public void GetSamplesAsync(short[] snd_Samples)
		{
			throw new NotSupportedException("Async is not available");
		}

		public void DiscardSamples()
		{
			_blip_L.Clear();
			_blip_R.Clear();
			snd_Master_Clock = 0;
		}

		public void DisposeSound()
		{
			_blip_L.Clear();
			_blip_R.Clear();
			_blip_L.Dispose();
			_blip_R.Dispose();
			_blip_L = null;
			_blip_R = null;
		}
	}
}