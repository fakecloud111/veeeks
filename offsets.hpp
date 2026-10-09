#pragma once

#include <cstdint>

namespace offsets {
    namespace vgk {
        inline uintptr_t ShadowRegions = 0x838F8; //0x82708
        inline uintptr_t ShadowRegionQ = 0x83910; //0x82720
        inline uintptr_t ShadowRegionB = 0x839C0; //0x827D0
    }

    namespace old_vgk {
        inline uintptr_t ShadowRegions = 0x82708;
        inline uintptr_t ShadowRegionQ = 0x82720;
        inline uintptr_t ShadowRegionB = 0x827D0;
    }

    inline uintptr_t FNamePool = 0xB600040; //0xB594040;
    inline uintptr_t FNameState = 0xB7DC100; //0xB770100

    inline uintptr_t GWorld = 0xB490930; //0xB424930
    inline uintptr_t OwningWord = 0x00C0;
    inline uintptr_t PersistentLevel = 0x0038;
    inline uintptr_t OwningGameInstance = 0x1D8;
	inline uintptr_t LocalPlayers = 0x0040;
    inline uintptr_t PlayerController = 0x0038;
    inline uintptr_t PlayerCameraManager = 0x0520;
	inline uintptr_t AcknowledgedPawn = 0x0510;
    inline uintptr_t PlayerState = 0x0480;
	inline uintptr_t CameraCache = 0x17B0;
    inline uintptr_t ActorArray = 0x00A0;
	inline uintptr_t ControlRotation = 0x04E0;

    inline uintptr_t UniqueID = 0x0030;
    inline uintptr_t Dormant = 0x0101;
	inline uintptr_t Mesh = 0x04E8;
    inline uintptr_t Inventory = 0x0BF0;
    inline uintptr_t CurrentEquippable = 0x0248;
	inline uintptr_t LastRenderTime = 0x047C;
	inline uintptr_t LastSubmitTime = 0x0478;
    inline uintptr_t CharacterMinimap = 0x14F0;
	inline uintptr_t ComponentToWorld = 0x02D0;
    inline uintptr_t RootComponent = 0x0288;
    inline uintptr_t TeamComponent = 0x06a8;
	inline uintptr_t DamageHandler = 0x0C50;
    inline uintptr_t TeamId = 0x00E8;
    inline uintptr_t bWasAlly = 0x0F09;
    inline uintptr_t RelativeLocation = 0x0170;
    inline uintptr_t RelativeRotation = 0x0188;
	inline uintptr_t Health = 0x01E0;
    inline uintptr_t Shield = 0x0124;
    inline uintptr_t MaxShield = 0x0128;
	inline uintptr_t ShieldType = 0x0118;
    inline uintptr_t BoneArray = 0x0730;
    inline uintptr_t BoneArrayCache = BoneArray + 0x10;
    inline uintptr_t SpikeTimer = 0x05A8;
	inline uintptr_t SpikeDefuseProgress = 0x05D0;
    inline uintptr_t DefuseSection = 0x05F0;
}