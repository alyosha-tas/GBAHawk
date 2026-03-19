#pragma once
// GBHawk.cpp : Defines the exported functions for the DLL.

#include "GBHawk.h"
#include "Core.h"
#include "LinkCore.h"

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace GBHawk;

#pragma region Core
// Create pointer to a core instance
GBHawk_EXPORT GBCore* GB_create()
{
	return new GBCore();
}

// free the memory from the core pointer
GBHawk_EXPORT void GB_destroy(GBCore* p)
{
	delete p->GB.Cart_RAM;
	delete p->GB.mapper_pntr;
	delete p->GB.ppu_pntr;

	std::free(p);
}

// load bios and basic into the core
GBHawk_EXPORT void GB_load_bios(GBCore* p, uint8_t* bios, bool cgb_flag, bool cgb_gba_flag)
{
	p->Load_BIOS(bios, cgb_flag, cgb_gba_flag);
}

// load a rom into the core
GBHawk_EXPORT void GB_load(GBCore* p, uint8_t* rom, uint32_t size, uint32_t mapper)
{
	p->Load_ROM(rom, size, mapper);
}

// set RTC
GBHawk_EXPORT void GB_set_rtc(GBCore* p, int32_t val, uint32_t param)
{
	p->Set_RTC(val, param);
}

// Create a default SRAM
GBHawk_EXPORT void GB_create_SRAM(GBCore* p, uint8_t* sram, uint32_t size)
{
	p->Create_SRAM(sram, size);
}

// load sram into the core
GBHawk_EXPORT void GB_load_SRAM(GBCore* p, uint8_t* sram, uint32_t size)
{
	p->Load_SRAM(sram, size);
}

GBHawk_EXPORT void GB_load_Palette(GBCore* p, bool palette)
{
	p->Set_Palette(palette);
}

GBHawk_EXPORT void GB_Sync_Domain_VBL(GBCore* p, bool on_vbl)
{
	p->Sync_Domain_VBL(on_vbl);
}

// reset the system
GBHawk_EXPORT void GB_Hard_Reset(GBCore* p)
{
	p->Hard_Reset();
}

// advance a frame
GBHawk_EXPORT bool GB_frame_advance(GBCore* p, uint8_t ctrl1, uint16_t accx, uint16_t accy, bool render, bool sound)
{
	return p->FrameAdvance(ctrl1, accx, accy, render, sound);
}

// subframe mode, input cycle is how long current input is applied for in the current frame
GBHawk_EXPORT bool GB_subframe_advance(GBCore* p, uint8_t ctrl1, uint16_t accx, uint16_t accy, bool render, bool sound, bool do_reset, uint32_t input_cycle)
{
	return p->SubFrameAdvance(ctrl1, accx, accy, render, sound, do_reset, input_cycle);
}

// send video data to external video provider
GBHawk_EXPORT void GB_get_video(GBCore* p, uint32_t* dest)
{
	p->GetVideo(dest);
}

// send audio data to external audio provider
GBHawk_EXPORT uint32_t GB_get_audio(GBCore* p, int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R)
{
	return p->GetAudio(dest_L, n_samp_L, dest_R, n_samp_R);
}

GBHawk_EXPORT void GB_setrumblecallback(GBCore* p, void (*callback)(bool))
{
	p->SetRumbleCallback(callback);
}

#pragma region State Save / Load

// save state
GBHawk_EXPORT void GB_save_state(GBCore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
GBHawk_EXPORT void GB_load_state(GBCore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

#pragma endregion

#pragma region Memory Domain Functions

GBHawk_EXPORT uint8_t GB_getsysbus(GBCore* p, uint32_t addr) {
	return p->GetSysBus(addr);
}

GBHawk_EXPORT uint8_t GB_getvram(GBCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetVRAM(addr, vbl_sync);
}

GBHawk_EXPORT uint8_t GB_getram(GBCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetRAM(addr, vbl_sync);
}

GBHawk_EXPORT uint8_t GB_gethram(GBCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetHRAM(addr, vbl_sync);
}

GBHawk_EXPORT uint8_t GB_getoam(GBCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetOAM(addr, vbl_sync);
}

GBHawk_EXPORT uint8_t GB_getregisters(GBCore* p, uint32_t addr)
{
	return p->GetRegisters(addr);
}

GBHawk_EXPORT uint8_t GB_getsram(GBCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetSRAM(addr, vbl_sync);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
GBHawk_EXPORT void GB_settracecallback(GBCore* p, void (*callback)(int))
{
	p->SetTraceCallback(callback);
}

// return the cpu trace header length
GBHawk_EXPORT int GB_getheaderlength(GBCore* p)
{
	return p->GetHeaderLength();
}

// return the cpu disassembly length
GBHawk_EXPORT int GB_getdisasmlength(GBCore* p)
{
	return p->GetDisasmLength();
}

// return the cpu register string length
GBHawk_EXPORT int GB_getregstringlength(GBCore* p)
{
	return p->GetRegStringLength();
}

// return the cpu trace header
GBHawk_EXPORT void GB_getheader(GBCore* p, char* h, int l)
{
	p->GetHeader(h, l);
}

// return the cpu register state
GBHawk_EXPORT void GB_getregisterstate(GBCore* p, char* r, int t, int l)
{
	p->GetRegisterState(r, t, l);
}

// return the cpu disassembly
GBHawk_EXPORT void GB_getdisassembly(GBCore* p, char* d, int t, int l)
{
	p->GetDisassembly(d, t, l);
}

#pragma endregion

#pragma region Messages

// set message callback
GBHawk_EXPORT void GB_setmessagecallback(GBCore* p, void (*callback)(int))
{
	p->SetMessageCallback(callback);
}

// get message
GBHawk_EXPORT void GB_getmessage(GBCore* p, char* d)
{
	p->GetMessage(d);
}

#pragma endregion

#pragma region PPU Viewer

// set tracer callback
GBHawk_EXPORT void* GB_get_ppu_pntrs(GBCore* p, int sel)
{
	return p->Get_PPU_Pointers(sel);
}

// set scanline callback
GBHawk_EXPORT void GB_setscanlinecallback(GBCore* p, void (*callback)(uint8_t), int sl)
{
	p->SetScanlineCallback(callback, sl);
}

GBHawk_EXPORT void GB_executescanlinecallback(GBCore* p)
{
	p->ExecuteScanlineCallback();
}


#pragma endregion

#pragma endregion

#pragma region Link Core
// Create pointer to a core instance
GBHawk_EXPORT GBLinkCore* GBLink_create(uint32_t Num_ROMs)
{
	return new GBLinkCore(Num_ROMs);
}

// free the memory from the core pointer
GBHawk_EXPORT void GBLink_destroy(GBLinkCore* p)
{
	if (p != nullptr)
	{
		for (int i = 0; i < p->Num_ROMs; i++)
		{
			delete p->GBL[i].GB.Cart_RAM;
			delete p->GBL[i].GB.mapper_pntr;
			delete p->GBL[i].GB.ppu_pntr;
		}

		delete p->GBL;
	}

	std::free(p);
}

// load bios and basic into the core
GBHawk_EXPORT void GBLink_load_bios(GBLinkCore* p, uint8_t* bios, bool gbcflag, bool gbc_gba_flag, uint32_t console_num)
{
	p->Load_BIOS(bios, gbcflag, gbc_gba_flag, console_num);
}

// load a rom into the core
GBHawk_EXPORT void GBLink_load(GBLinkCore* p, uint8_t* rom, uint32_t size, uint32_t mapper, uint32_t console_num)
{
	p->Load_ROM(rom, size, mapper, console_num);
}

// Create a default SRAM
GBHawk_EXPORT void GBLink_create_SRAM(GBLinkCore* p, uint8_t* sram, uint32_t size, uint32_t console_num)
{
	p->Create_SRAM(sram, size, console_num);
}

// load sram into the core
GBHawk_EXPORT void GBLink_load_SRAM(GBLinkCore* p, uint8_t* sram, uint32_t size, uint32_t num)
{
	p->Load_SRAM(sram, size, num);
}

// reset the system
GBHawk_EXPORT void GBLink_Hard_Reset(GBLinkCore* p, uint32_t console_num)
{
	p->Hard_Reset(console_num);
}

// advance a frame
GBHawk_EXPORT bool GBLink_frame_advance(GBLinkCore* p, uint16_t ctrl_0, uint16_t accx_0, uint16_t accy_0,  bool render_0, bool sound_0,
														  uint16_t ctrl_1, uint16_t accx_1, uint16_t accy_1, bool render_1, bool sound_1,
														  bool l_reset, bool r_reset)
{
	return p->FrameAdvance(ctrl_0, accx_0, accy_0, render_0, sound_0, ctrl_1, accx_1, accy_1, render_1, sound_1, l_reset, r_reset);
}

// send video data to external video provider
GBHawk_EXPORT void GBLink_get_video(GBLinkCore* p, uint32_t* dest, uint32_t num)
{
	p->GetVideo(dest, num);
}

// send audio data to external audio provider
GBHawk_EXPORT uint32_t GBLink_get_audio(GBLinkCore* p, int32_t* dest_L_0, int32_t* n_samp_L_0, int32_t* dest_R_0, int32_t* n_samp_R_0, 
													   int32_t* dest_L_1, int32_t* n_samp_L_1, int32_t* dest_R_1, int32_t* n_samp_R_1, bool* enables)
{
	return p->GetAudio(dest_L_0, n_samp_L_0, dest_R_0, n_samp_R_0, dest_L_1, n_samp_L_1, dest_R_1, n_samp_R_1, enables);
}

#pragma region State Save / Load

// save state
GBHawk_EXPORT void GBLink_save_state(GBLinkCore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
GBHawk_EXPORT void GBLink_load_state(GBLinkCore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

#pragma endregion

#pragma region Memory Domain Functions

GBHawk_EXPORT uint8_t GBLink_getsysbus(GBLinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetSysBus(addr, num);
}

GBHawk_EXPORT uint8_t GBLink_getvram(GBLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetVRAM(addr, vbl_sync, num);
}

GBHawk_EXPORT uint8_t GBLink_getram(GBLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetRAM(addr, vbl_sync, num);
}

GBHawk_EXPORT uint8_t GBLink_gethram(GBLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetHRAM(addr, vbl_sync, num);
}

GBHawk_EXPORT uint8_t GBLink_getoam(GBLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetOAM(addr, vbl_sync, num);
}

GBHawk_EXPORT uint8_t GBLink_getregisters(GBLinkCore* p, uint32_t addr, uint32_t num)
{
	return p->GetRegisters(addr, num);
}

GBHawk_EXPORT uint8_t GBLink_getsram(GBLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetSRAM(addr, vbl_sync, num);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
GBHawk_EXPORT void GBLink_settracecallback(GBLinkCore* p, void (*callback)(int), uint32_t num) {
	p->SetTraceCallback(callback, num);
}

// return the cpu trace header length
GBHawk_EXPORT int GBLink_getheaderlength(GBLinkCore* p) {
	return p->GetHeaderLength();
}

// return the cpu disassembly length
GBHawk_EXPORT int GBLink_getdisasmlength(GBLinkCore* p) {
	return p->GetDisasmLength();
}

// return the cpu register string length
GBHawk_EXPORT int GBLink_getregstringlength(GBLinkCore* p) {
	return p->GetRegStringLength();
}

// return the cpu trace header
GBHawk_EXPORT void GBLink_getheader(GBLinkCore* p, char* h, int l) {
	p->GetHeader(h, l);
}

// return the cpu register state
GBHawk_EXPORT void GBLink_getregisterstate(GBLinkCore* p, char* r, int t, int l, uint32_t num) {
	p->GetRegisterState(r, t, l, num);
}

// return the cpu disassembly
GBHawk_EXPORT void GBLink_getdisassembly(GBLinkCore* p, char* d, int t, int l, uint32_t num) {
	p->GetDisassembly(d, t, l, num);
}

#pragma endregion

#pragma endregion