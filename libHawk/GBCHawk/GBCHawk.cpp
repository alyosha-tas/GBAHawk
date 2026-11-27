#pragma once
// GBCHawk.cpp : Defines the exported functions for the DLL.

#include "GBCHawk.h"
#include "Core.h"
#include "LinkCore.h"

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace GBCHawk;

#pragma region Core
// Create pointer to a core instance
GBCHawk_EXPORT GBCCore* GBC_create()
{
	return new GBCCore();
}

// free the memory from the core pointer
GBCHawk_EXPORT void GBC_destroy(GBCCore* p)
{
	delete p->GBC.Cart_RAM;
	delete p->GBC.mapper_pntr;
	delete p->GBC.ppu_pntr;

	std::free(p);
}

// load bios and basic into the core
GBCHawk_EXPORT void GBC_load_bios(GBCCore* p, uint8_t* bios, bool cgb_gba_flag)
{
	p->Load_BIOS(bios, cgb_gba_flag);
}

// load a rom into the core
GBCHawk_EXPORT void GBC_load(GBCCore* p, uint8_t* rom, uint32_t size, uint32_t mapper)
{
	p->Load_ROM(rom, size, mapper);
}

// set RTC
GBCHawk_EXPORT void GBC_set_rtc(GBCCore* p, int32_t val, uint32_t param)
{
	p->Set_RTC(val, param);
}

// Create a default SRAM
GBCHawk_EXPORT void GBC_create_SRAM(GBCCore* p, uint8_t* sram, uint32_t size)
{
	p->Create_SRAM(sram, size);
}

// load sram into the core
GBCHawk_EXPORT void GBC_load_SRAM(GBCCore* p, uint8_t* sram, uint32_t size)
{
	p->Load_SRAM(sram, size);
}

GBCHawk_EXPORT void GBC_Sync_Domain_VBL(GBCCore* p, bool on_vbl)
{
	p->Sync_Domain_VBL(on_vbl);
}

// reset the system
GBCHawk_EXPORT void GBC_Hard_Reset(GBCCore* p)
{
	p->Hard_Reset();
}

// advance a frame
GBCHawk_EXPORT bool GBC_frame_advance(GBCCore* p, uint8_t ctrl1, uint16_t accx, uint16_t accy, bool render, bool sound)
{
	return p->FrameAdvance(ctrl1, accx, accy, render, sound);
}

// subframe mode, input cycle is how long current input is applied for in the current frame
GBCHawk_EXPORT bool GBC_subframe_advance(GBCCore* p, uint8_t ctrl1, uint16_t accx, uint16_t accy, bool render, bool sound, bool do_reset, uint32_t input_cycle)
{
	return p->SubFrameAdvance(ctrl1, accx, accy, render, sound, do_reset, input_cycle);
}

// send video data to external video provider
GBCHawk_EXPORT void GBC_get_video(GBCCore* p, uint32_t* dest)
{
	p->GetVideo(dest);
}

// send audio data to external audio provider
GBCHawk_EXPORT uint32_t GBC_get_audio(GBCCore* p, int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R)
{
	return p->GetAudio(dest_L, n_samp_L, dest_R, n_samp_R);
}

GBCHawk_EXPORT void GBC_setrumblecallback(GBCCore* p, void (*callback)(bool))
{
	p->SetRumbleCallback(callback);
}

GBCHawk_EXPORT uint64_t GBC_get_cycles(GBCCore* p, bool cycle_type)
{
	return p->GetCycles(cycle_type);
}

#pragma region State Save / Load

// save state
GBCHawk_EXPORT void GBC_save_state(GBCCore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
GBCHawk_EXPORT void GBC_load_state(GBCCore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

#pragma endregion

#pragma region Memory Domain Functions

GBCHawk_EXPORT uint8_t GBC_getsysbus(GBCCore* p, uint32_t addr) {
	return p->GetSysBus(addr);
}

GBCHawk_EXPORT uint8_t GBC_getvram(GBCCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetVRAM(addr, vbl_sync);
}

GBCHawk_EXPORT uint8_t GBC_getram(GBCCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetRAM(addr, vbl_sync);
}

GBCHawk_EXPORT uint8_t GBC_gethram(GBCCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetHRAM(addr, vbl_sync);
}

GBCHawk_EXPORT uint8_t GBC_getoam(GBCCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetOAM(addr, vbl_sync);
}

GBCHawk_EXPORT uint8_t GBC_getregisters(GBCCore* p, uint32_t addr)
{
	return p->GetRegisters(addr);
}

GBCHawk_EXPORT uint8_t GBC_getsram(GBCCore* p, uint32_t addr, bool vbl_sync)
{
	return p->GetSRAM(addr, vbl_sync);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
GBCHawk_EXPORT void GBC_settracecallback(GBCCore* p, void (*callback)(int))
{
	p->SetTraceCallback(callback);
}

// return the cpu trace header length
GBCHawk_EXPORT int GBC_getheaderlength(GBCCore* p)
{
	return p->GetHeaderLength();
}

// return the cpu disassembly length
GBCHawk_EXPORT int GBC_getdisasmlength(GBCCore* p)
{
	return p->GetDisasmLength();
}

// return the cpu register string length
GBCHawk_EXPORT int GBC_getregstringlength(GBCCore* p)
{
	return p->GetRegStringLength();
}

// return the cpu trace header
GBCHawk_EXPORT void GBC_getheader(GBCCore* p, char* h, int l)
{
	p->GetHeader(h, l);
}

// return the cpu register state
GBCHawk_EXPORT void GBC_getregisterstate(GBCCore* p, char* r, int t, int l)
{
	p->GetRegisterState(r, t, l);
}

// return the cpu disassembly
GBCHawk_EXPORT void GBC_getdisassembly(GBCCore* p, char* d, int t, int l)
{
	p->GetDisassembly(d, t, l);
}

#pragma endregion

#pragma region Messages

// set message callback
GBCHawk_EXPORT void GBC_setmessagecallback(GBCCore* p, void (*callback)(int))
{
	p->SetMessageCallback(callback);
}

// get message
GBCHawk_EXPORT void GBC_getmessage(GBCCore* p, char* d)
{
	p->GetMessage(d);
}

#pragma endregion

#pragma region PPU Viewer

// set tracer callback
GBCHawk_EXPORT void* GBC_get_ppu_pntrs(GBCCore* p, int sel)
{
	return p->Get_PPU_Pointers(sel);
}

// set scanline callback
GBCHawk_EXPORT void GBC_setscanlinecallback(GBCCore* p, void (*callback)(uint8_t), int sl)
{
	p->SetScanlineCallback(callback, sl);
}

GBCHawk_EXPORT void GBC_executescanlinecallback(GBCCore* p)
{
	p->ExecuteScanlineCallback();
}


#pragma endregion

#pragma endregion

#pragma region Link Core
// Create pointer to a core instance
GBCHawk_EXPORT GBCLinkCore* GBCLink_create(uint32_t Num_ROMs)
{
	return new GBCLinkCore(Num_ROMs);
}

// free the memory from the core pointer
GBCHawk_EXPORT void GBCLink_destroy(GBCLinkCore* p)
{
	if (p != nullptr)
	{
		for (int i = 0; i < p->Num_ROMs; i++)
		{
			delete p->GBCL[i].GBC.Cart_RAM;
			delete p->GBCL[i].GBC.mapper_pntr;
			delete p->GBCL[i].GBC.ppu_pntr;
		}

		delete[] p->GBCL;
	}

	std::free(p);
}

// load bios and basic into the core
GBCHawk_EXPORT void GBCLink_load_bios(GBCLinkCore* p, uint8_t* bios, bool gbc_gba_flag, uint32_t console_num)
{
	p->Load_BIOS(bios, gbc_gba_flag, console_num);
}

// load a rom into the core
GBCHawk_EXPORT void GBCLink_load(GBCLinkCore* p, uint8_t* rom, uint32_t size, uint32_t mapper, uint32_t console_num)
{
	p->Load_ROM(rom, size, mapper, console_num);
}

// set RTC
GBCHawk_EXPORT void GBCLink_set_rtc(GBCLinkCore* p, int32_t val, uint32_t param, uint32_t console_num)
{
	p->Set_RTC(val, param, console_num);
}

// Create a default SRAM
GBCHawk_EXPORT void GBCLink_create_SRAM(GBCLinkCore* p, uint8_t* sram, uint32_t size, uint32_t console_num)
{
	p->Create_SRAM(sram, size, console_num);
}

// load sram into the core
GBCHawk_EXPORT void GBCLink_load_SRAM(GBCLinkCore* p, uint8_t* sram, uint32_t size, uint32_t num)
{
	p->Load_SRAM(sram, size, num);
}

GBCHawk_EXPORT void GBCLink_Sync_Domain_VBL(GBCLinkCore* p, bool on_vbl, uint32_t console_num)
{
	p->Sync_Domain_VBL(on_vbl, console_num);
}

// reset the system
GBCHawk_EXPORT void GBCLink_Hard_Reset(GBCLinkCore* p)
{
	p->Hard_Reset();
}

// advance a frame
GBCHawk_EXPORT bool GBCLink_frame_advance(GBCLinkCore* p, uint8_t* ctrls, uint16_t* accxs, uint16_t* accys, bool* renders, bool* sounds, bool* resets)
{
	return p->FrameAdvance(ctrls, accxs, accys, renders, sounds, resets);
}

// send video data to external video provider
GBCHawk_EXPORT void GBCLink_get_video(GBCLinkCore* p, uint32_t* dest, uint32_t num)
{
	p->GetVideo(dest, num);
}

// send audio data to external audio provider
GBCHawk_EXPORT uint32_t GBCLink_get_audio(GBCLinkCore* p, int32_t* dest_L_0, int32_t* n_samp_L_0, int32_t* dest_R_0, int32_t* n_samp_R_0, 
													   int32_t* dest_L_1, int32_t* n_samp_L_1, int32_t* dest_R_1, int32_t* n_samp_R_1, bool* enables)
{
	return p->GetAudio(dest_L_0, n_samp_L_0, dest_R_0, n_samp_R_0, dest_L_1, n_samp_L_1, dest_R_1, n_samp_R_1, enables);
}

GBCHawk_EXPORT void GBCLink_change_linking(GBCLinkCore* p, bool link_status, uint32_t link_type)
{
	p->Change_Linking(link_status, link_type);
}

#pragma region State Save / Load

// save state
GBCHawk_EXPORT void GBCLink_save_state(GBCLinkCore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
GBCHawk_EXPORT void GBCLink_load_state(GBCLinkCore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

#pragma endregion

#pragma region Memory Domain Functions

GBCHawk_EXPORT uint8_t GBCLink_getsysbus(GBCLinkCore* p, uint32_t addr, uint32_t num) {
	return p->GetSysBus(addr, num);
}

GBCHawk_EXPORT uint8_t GBCLink_getvram(GBCLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetVRAM(addr, vbl_sync, num);
}

GBCHawk_EXPORT uint8_t GBCLink_getram(GBCLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetRAM(addr, vbl_sync, num);
}

GBCHawk_EXPORT uint8_t GBCLink_gethram(GBCLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetHRAM(addr, vbl_sync, num);
}

GBCHawk_EXPORT uint8_t GBCLink_getoam(GBCLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetOAM(addr, vbl_sync, num);
}

GBCHawk_EXPORT uint8_t GBCLink_getregisters(GBCLinkCore* p, uint32_t addr, uint32_t num)
{
	return p->GetRegisters(addr, num);
}

GBCHawk_EXPORT uint8_t GBCLink_getsram(GBCLinkCore* p, uint32_t addr, bool vbl_sync, uint32_t num) {
	return p->GetSRAM(addr, vbl_sync, num);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
GBCHawk_EXPORT void GBCLink_settracecallback(GBCLinkCore* p, void (*callback)(int), uint32_t num) {
	p->SetTraceCallback(callback, num);
}

// return the cpu trace header length
GBCHawk_EXPORT int GBCLink_getheaderlength(GBCLinkCore* p) {
	return p->GetHeaderLength();
}

// return the cpu disassembly length
GBCHawk_EXPORT int GBCLink_getdisasmlength(GBCLinkCore* p) {
	return p->GetDisasmLength();
}

// return the cpu register string length
GBCHawk_EXPORT int GBCLink_getregstringlength(GBCLinkCore* p) {
	return p->GetRegStringLength();
}

// return the cpu trace header
GBCHawk_EXPORT void GBCLink_getheader(GBCLinkCore* p, char* h, int l) {
	p->GetHeader(h, l);
}

// return the cpu register state
GBCHawk_EXPORT void GBCLink_getregisterstate(GBCLinkCore* p, char* r, int t, int l, uint32_t num) {
	p->GetRegisterState(r, t, l, num);
}

// return the cpu disassembly
GBCHawk_EXPORT void GBCLink_getdisassembly(GBCLinkCore* p, char* d, int t, int l, uint32_t num) {
	p->GetDisassembly(d, t, l, num);
}

#pragma endregion

#pragma region Messages

// set message callback
GBCHawk_EXPORT void GBCLink_setmessagecallback(GBCLinkCore* p, void (*callback)(int))
{
	p->SetMessageCallback(callback);
}

// get message
GBCHawk_EXPORT void GBCLink_getmessage(GBCLinkCore* p, char* d)
{
	p->GetMessage(d);
}

#pragma endregion

#pragma endregion