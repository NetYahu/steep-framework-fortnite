#pragma once

#include <cheat/unreal/types.h>

// skeleton indices for the Fortnite player mesh, ported from
// FortniteESPBase (unreal/enums.hh).
namespace unreal {
    enum class e_fort_skeleton : i32 {
        root = 0,
        attach = 1,
        pelvis = 2,
        spine_01 = 3,
        spine_02 = 4,
        spine_03 = 5,
        spine_04 = 6,
        spine_05 = 7,
        clavicle_l = 8,
        upperarm_l = 9,
        lowerarm_l = 10,
        hand_l = 11,
        clavicle_r = 37,
        upperarm_r = 38,
        lowerarm_r = 39,
        hand_r = 40,
        neck_01 = 66,
        neck_02 = 67,
        chest = 68,
        head = 110,
        thigh_l = 71,
        calf_l = 72,
        foot_l = 86,
        thigh_r = 78,
        calf_r = 79,
        foot_r = 87,
        weapon_l = 31,
        weapon_r = 60,
    };
}