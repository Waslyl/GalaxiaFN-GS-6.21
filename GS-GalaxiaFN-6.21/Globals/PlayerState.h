#pragma once

namespace PlayerState
{
	void InitPlayerState()
	{
		static auto FortPlayerStateAthenaDefault = FindObjectFast<AFortPlayerStateAthena>("/Script/FortniteGame.Default__FortPlayerStateAthena");

		FN_LOG(LogInit, Log, "InitPlayerState Success!");
	}
}