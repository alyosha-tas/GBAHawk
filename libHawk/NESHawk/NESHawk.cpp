// NESHawk.cpp : Defines the exported functions for the DLL.

#include "NESHawk.h"
#include "Core.h"

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace NESHawk;

#pragma region Core
// Create pointer to a core instance
NESHawk_EXPORT NESCore* NES_create()
{
	return new NESCore();
}

// free the memory from the core pointer
NESHawk_EXPORT void NES_destroy(NESCore* p)
{
	delete p->NES.Cart_RAM;

	std::free(p);
}

// load a rom into the core
NESHawk_EXPORT void NES_load(NESCore* p, uint8_t* rom, uint32_t size, uint8_t* header, bool mmc3_old_irq, bool mapper_bus_conflicts, bool apu_test_regs, bool cpu_zero_reset, uint8_t special_flag)
{
	p->Load_ROM(rom, size, header, mmc3_old_irq, mapper_bus_conflicts, apu_test_regs, cpu_zero_reset, special_flag);
}

// Create a default SRAM
NESHawk_EXPORT void NES_create_SRAM(NESCore* p, uint8_t* sram, uint32_t size)
{
	p->Create_SRAM(sram, size);
}

// load sram into the core
NESHawk_EXPORT void NES_load_SRAM(NESCore* p, uint8_t* sram, uint32_t size)
{
	p->Load_SRAM(sram, size);
}

// load palette into the core
NESHawk_EXPORT void NES_load_Palette(NESCore* p, uint8_t* palette)
{
	p->Load_Palette(palette);
}

// reset the system
NESHawk_EXPORT void NES_Hard_Reset(NESCore* p)
{
	p->Hard_Reset();
}

// soft reset the system
NESHawk_EXPORT void NES_Soft_Reset(NESCore* p)
{
	p->Soft_Reset();
}

// advance a frame
NESHawk_EXPORT bool NES_frame_advance(NESCore* p, bool render, bool sound)
{
	return p->FrameAdvance( render, sound);
}

// get cpu clocks
NESHawk_EXPORT uint64_t NES_get_cycles(NESCore* p)
{
	return p->NES.TotalExecutedCycles;
}

// advance a frame and possibly subframe reset
NESHawk_EXPORT bool NES_subframe_advance(NESCore* p, bool render, bool sound, bool do_reset, uint32_t reset_cycle)
{
	return p->SubFrameAdvance(render, sound, do_reset, reset_cycle);
}

// get clocks needed for timing subframe movies
NESHawk_EXPORT uint64_t NES_subframe_cycles(NESCore* p)
{
	return p->NES.Total_CPU_Clock_Cycles;
}

NESHawk_EXPORT uint8_t NES_getzapperstate(NESCore* p, int zapper_x, int zapper_y)
{
	return p->GetZapperState(zapper_x, zapper_y);
}

// set controller read callback
NESHawk_EXPORT void NES_setcontrollercallback(NESCore* p, uint8_t (*callback)(bool)) {
	p->SetControllerCallback(callback);
}

// set controller strobe callback
NESHawk_EXPORT void NES_setstrobecallback(NESCore* p, void(*callback)(uint8_t, uint8_t)) {
	p->SetStrobeCallback(callback);
}

// send video data to external video provider
NESHawk_EXPORT void NES_get_video(NESCore* p, uint32_t* dest)
{
	p->GetVideo(dest);
}

// send audio data to external audio provider
NESHawk_EXPORT uint32_t NES_get_audio(NESCore* p, int32_t* dest, int32_t* n_samp)
{
	return p->GetAudio(dest, n_samp);
}

#pragma region State Save / Load

// save state
NESHawk_EXPORT void NES_save_state(NESCore* p, uint8_t* saver)
{
	p->SaveState(saver);
}

// load state
NESHawk_EXPORT void NES_load_state(NESCore* p, uint8_t* loader)
{
	p->LoadState(loader);
}

#pragma endregion

#pragma region Memory Domain Functions

NESHawk_EXPORT uint8_t NES_getsysbus(NESCore* p, uint32_t addr) {
	return p->GetSysBus(addr);
}

NESHawk_EXPORT uint8_t NES_getvram(NESCore* p, uint32_t addr) {
	return p->GetVRAM(addr);
}

NESHawk_EXPORT uint8_t NES_getchrrom(NESCore* p, uint32_t addr) {
	return p->GetCHR_ROM(addr);
}

NESHawk_EXPORT uint8_t NES_getram(NESCore* p, uint32_t addr) {
	return p->GetRAM(addr);
}

NESHawk_EXPORT uint8_t NES_getrom(NESCore* p, uint32_t addr) {
	return p->GetROM(addr);
}

NESHawk_EXPORT uint8_t NES_getoam(NESCore* p, uint32_t addr) {
	return p->GetOAM(addr);
}

NESHawk_EXPORT uint8_t NES_getpalram(NESCore* p, uint32_t addr) {
	return p->GetPALRAM(addr);
}

NESHawk_EXPORT uint8_t NES_getsram(NESCore* p, uint32_t addr) {
	return p->GetSRAM(addr);
}

#pragma endregion

#pragma region Tracer

// set tracer callback
NESHawk_EXPORT void NES_settracecallback(NESCore* p, void (*callback)(int)) {
	p->SetTraceCallback(callback);
}

// return the cpu trace header length
NESHawk_EXPORT int NES_getheaderlength(NESCore* p) {
	return p->GetHeaderLength();
}

// return the cpu disassembly length
NESHawk_EXPORT int NES_getdisasmlength(NESCore* p) {
	return p->GetDisasmLength();
}

// return the cpu register string length
NESHawk_EXPORT int NES_getregstringlength(NESCore* p) {
	return p->GetRegStringLength();
}

// return the cpu trace header
NESHawk_EXPORT void NES_getheader(NESCore* p, char* h, int l) {
	p->GetHeader(h, l);
}

// return the cpu register state
NESHawk_EXPORT void NES_getregisterstate(NESCore* p, char* r, int t, int l) {
	p->GetRegisterState(r, t, l);
}

// return the cpu disassembly
NESHawk_EXPORT void NES_getdisassembly(NESCore* p, char* d, int t, int l) {
	p->GetDisassembly(d, t, l);
}

#pragma endregion

#pragma region Messages

// set message callback
NESHawk_EXPORT void NES_setmessagecallback(NESCore* p, void (*callback)(int)) {
	p->SetMessageCallback(callback);
}

// get message
NESHawk_EXPORT void NES_getmessage(NESCore* p, char* d) {
	p->GetMessage(d);
}

// setinput callback
NESHawk_EXPORT void NES_setinputpollcallback(NESCore* p, void (*callback)()) {
	p->SetInputPollCallback(callback);
}


#pragma endregion

#pragma region PPU Viewer

// get values from ppu

NESHawk_EXPORT bool NES_get_ppu_vals(NESCore* p, uint32_t sel)
{
	return p->Get_PPU_Values(sel);
}

// get values from ppu bus
NESHawk_EXPORT uint8_t NES_get_ppu_bus(NESCore* p, uint32_t addr)
{
	return p->Get_PPU_Bus_Peek(addr);
}

// get values from ppu bus as seen by board
NESHawk_EXPORT uint8_t NES_get_board_peek_ppu(NESCore* p, uint32_t addr)
{
	return p->Get_PPU_Board_Peek(addr);
}

// get pointers from ppu
NESHawk_EXPORT uint8_t* NES_get_ppu_pntrs(NESCore* p, int sel)
{
	return p->Get_PPU_Pointers(sel);
}

// set nametable viewer callback
NESHawk_EXPORT void NES_setntvcallback(NESCore* p, void (*callback)(void), int sl) {
	p->SetNTVCallback(callback, sl);
}

// set ppu viewer callback
NESHawk_EXPORT void NES_setppucallback(NESCore* p, void (*callback)(void), int sl) {
	p->SetPPUCallback(callback, sl);
}

#pragma endregion

#pragma endregion