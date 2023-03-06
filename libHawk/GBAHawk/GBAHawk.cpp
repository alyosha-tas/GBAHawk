// GBAHawk.cpp : Defines the exported functions for the DLL.

#include "GBAHawk.h"
#include "Core.h"

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace GBAHawk;

#pragma region Core
// Create pointer to a core instance
GBAHawk_EXPORT GBACore* GBA_create()
{
	return new GBACore();
}

// free the memory from the core pointer
GBAHawk_EXPORT void GBA_destroy(GBACore* p)
{
	delete p->GBA.Cart_RAM;

	std::free(p);
}

// load bios and basic into the core
GBAHawk_EXPORT void GBA_load_bios(GBACore* p, uint8_t* bios)
{
	p->Load_BIOS(bios);
}

// load a rom into the core
GBAHawk_EXPORT void GBA_load(GBACore* p, uint8_t* rom, uint32_t size, uint32_t mapper)
{
	p->Load_ROM(rom, size, mapper);
}

// Create a default SRAM
GBAHawk_EXPORT void GBA_create_SRAM(GBACore* p, uint8_t* sram, uint32_t size)
{
	p->Create_SRAM(sram, size);
}

// load sram into the core
GBAHawk_EXPORT void GBA_load_SRAM(GBACore* p, uint8_t* sram, uint32_t size)
{
	p->Load_SRAM(sram, size);
}

// reset the system
GBAHawk_EXPORT void GBA_Hard_Reset(GBACore* p)
{
	p->Hard_Reset();
}

// advance a frame
GBAHawk_EXPORT bool GBA_frame_advance(GBACore* p, uint16_t ctrl1, uint16_t accx, uint16_t accy, uint8_t solar, bool render, bool sound)
{
	return p->FrameAdvance(ctrl1, accx, accy, solar, render, sound);
}

// send video data to external video provider
GBAHawk_EXPORT void GBA_get_video(GBACore* p, uint32_t* dest)
{
	p->GetVideo(dest);
}

// send audio data to external audio provider
GBAHawk_EXPORT uint32_t GBA_get_audio(GBACore* p, int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R)
{
	return p->GetAudio(dest_L, n_samp_L, dest_R, n_samp_R);
}

#pragma region State Save / Load

// save state
GBAHawk_EXPORT void GBA_save_state(GBACore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
GBAHawk_EXPORT void GBA_load_state(GBACore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

#pragma endregion

#pragma region Memory Domain Functions

GBAHawk_EXPORT uint8_t GBA_getsysbus(GBACore* p, uint32_t addr) {
	return p->GetSysBus(addr);
}

GBAHawk_EXPORT uint8_t GBA_getvram(GBACore* p, uint32_t addr) {
	return p->GetVRAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getwram(GBACore* p, uint32_t addr) {
	return p->GetWRAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getiwram(GBACore* p, uint32_t addr) {
	return p->GetIWRAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getoam(GBACore* p, uint32_t addr) {
	return p->GetOAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getpalram(GBACore* p, uint32_t addr) {
	return p->GetPALRAM(addr);
}

GBAHawk_EXPORT uint8_t GBA_getsram(GBACore* p, uint32_t addr) {
	return p->GetSRAM(addr);
}

#pragma endregion


#pragma region Tracer

// set tracer callback
GBAHawk_EXPORT void GBA_settracecallback(GBACore* p, void (*callback)(int)) {
	p->SetTraceCallback(callback);
}

// return the cpu trace header length
GBAHawk_EXPORT int GBA_getheaderlength(GBACore* p) {
	return p->GetHeaderLength();
}

// return the cpu disassembly length
GBAHawk_EXPORT int GBA_getdisasmlength(GBACore* p) {
	return p->GetDisasmLength();
}

// return the cpu register string length
GBAHawk_EXPORT int GBA_getregstringlength(GBACore* p) {
	return p->GetRegStringLength();
}

// return the cpu trace header
GBAHawk_EXPORT void GBA_getheader(GBACore* p, char* h, int l) {
	p->GetHeader(h, l);
}

// return the cpu register state
GBAHawk_EXPORT void GBA_getregisterstate(GBACore* p, char* r, int t, int l) {
	p->GetRegisterState(r, t, l);
}

// return the cpu disassembly
GBAHawk_EXPORT void GBA_getdisassembly(GBACore* p, char* d, int t, int l) {
	p->GetDisassembly(d, t, l);
}

#pragma endregion

#pragma region Messages

// set message callback
GBAHawk_EXPORT void GBA_setmessagecallback(GBACore* p, void (*callback)(void)) {
	p->SetMessageCallback(callback);
}

// get message
GBAHawk_EXPORT void GBA_getmessage(GBACore* p, char* d, int l) {
	p->GetMessage(d, l);
}

#pragma endregion

#pragma region PPU Viewer

// set tracer callback
GBAHawk_EXPORT uint8_t* GBA_get_ppu_pntrs(GBACore* p, int sel)
{
	return p->Get_PPU_Pointers(sel);
}

// set scanline callback
GBAHawk_EXPORT void GBA_setscanlinecallback(GBACore* p, void (*callback)(void), int sl) {
	p->SetScanlineCallback(callback, sl);
}

#pragma endregion