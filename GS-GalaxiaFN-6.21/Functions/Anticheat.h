#pragma once
#include <map>
#include <xtree>
#include <thread>
#include <mutex>

// Ne pas oublier que Snoowz est un soumis

std::mutex ACWebhookMutex;

enum ACLogLevel : int32
{
	ACLog,
	ACWarning,
	ACError
};

void WriteLogAntiCheat(ACLogLevel level, const char* format, ...)
{
	const char* levelStr;

	switch (level) {
		case ACLogLevel::ACLog: levelStr = "Info"; break;
		case ACLogLevel::ACWarning: levelStr = "Warning"; break;
		case ACLogLevel::ACError: levelStr = "Error"; break;
		default: levelStr = "Unknown"; break;
	}

	printf("LogGalaxiaAntiCheat: %s: ", levelStr);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}

#define AC_LOG(Level, ...) WriteLogAntiCheat(Level, __VA_ARGS__)

struct FAntiCheatDetection
{
	enum class EAntiCheatDetection : uint8
	{
		None = 0,
		Low = 1,
		Medium = 2,
		High = 3,
		Critical = 4,
		Banned = 5,
		EAntiCheatDetection_MAX
	};

	int32 AntiCheatDetection;
	EAntiCheatDetection Detection;

	FAntiCheatDetection()
		: AntiCheatDetection(0)
		, Detection(EAntiCheatDetection::None)
	{
	}

public:
	static std::string ToString(EAntiCheatDetection AntiCheatDetection)
	{
		switch (AntiCheatDetection)
		{
			case EAntiCheatDetection::None:
				return "None";
			case EAntiCheatDetection::Low:
				return "Low";
			case EAntiCheatDetection::Medium:
				return "Medium";
			case EAntiCheatDetection::High:
				return "High";
			case EAntiCheatDetection::Critical:
				return "Critical";
			case EAntiCheatDetection::Banned:
				return "Banned";
			default:
				return "Error";
		}
	}
};

struct FAntiCheatZipline
{
	bool bOldJumped;

	FAntiCheatZipline()
		: bOldJumped(false)
	{
	}
};

class UAntiCheatGalaxia
{
public:
	static inline std::unordered_map<AFortPlayerController*, UAntiCheatGalaxia*> AntiCheatGalaxiaMap;

	AFortPlayerController* PlayerController;
	std::string PlayerName;
	FAntiCheatDetection AntiCheatDetection;
	FAntiCheatZipline AntiCheatZipline;

public:
	UAntiCheatGalaxia()
		: PlayerController(nullptr)
		, PlayerName("")
		, AntiCheatDetection(FAntiCheatDetection())
		, AntiCheatZipline(FAntiCheatZipline())
	{
	}

	int32 GetMaxDataNum() const
	{
		return 25;
	}

	void SendWebhook(const std::string& Title, const std::string& Content)
	{
		std::thread([Title, Content]()
			{
				std::lock_guard<std::mutex> Lock(ACWebhookMutex);
				AntiCheatWebHook.send_embed_Anticheat(Title, Content, 16711680);
			}).detach();
	}

	void SetDetection(FAntiCheatDetection::EAntiCheatDetection InDetection)
	{
		AntiCheatDetection.Detection = InDetection;

		const std::string& AntiCheatDetectionString = FAntiCheatDetection::ToString(InDetection);

		AC_LOG(ACLogLevel::ACWarning, "UAntiCheatGalaxia::SetDetection - Le joueur %s vient de passer en detection [%s]", PlayerName.c_str(), AntiCheatDetectionString.c_str());

		const std::string& Content = "Le joueur: [" + PlayerName + "] vient de passer en detection: [" + AntiCheatDetectionString + "]";
		SendWebhook("Anti Cheat Galaxia", Content);

		if (InDetection == FAntiCheatDetection::EAntiCheatDetection::Banned)
		{
			KickByAntiCheat("Detection passer en Banned.");
		}
	}

	void AddDetection(int32 InAntiCheatDetectionToAdd)
	{
		int32 OldAntiCheatDetection = AntiCheatDetection.AntiCheatDetection;

		AntiCheatDetection.AntiCheatDetection += InAntiCheatDetectionToAdd;

		FAntiCheatDetection::EAntiCheatDetection NewInDetection = FAntiCheatDetection::EAntiCheatDetection::None;
		if (AntiCheatDetection.AntiCheatDetection >= 10 && OldAntiCheatDetection < 10)
		{
			NewInDetection = FAntiCheatDetection::EAntiCheatDetection::Low;
		}
		else if (AntiCheatDetection.AntiCheatDetection >= 20 && OldAntiCheatDetection < 20)
		{
			NewInDetection = FAntiCheatDetection::EAntiCheatDetection::Medium;
		}
		else if (AntiCheatDetection.AntiCheatDetection >= 30 && OldAntiCheatDetection < 30)
		{
			NewInDetection = FAntiCheatDetection::EAntiCheatDetection::High;
		}
		else if (AntiCheatDetection.AntiCheatDetection >= 40 && OldAntiCheatDetection < 40)
		{
			NewInDetection = FAntiCheatDetection::EAntiCheatDetection::Critical;
		}
		else if (AntiCheatDetection.AntiCheatDetection >= 50)
		{
			NewInDetection = FAntiCheatDetection::EAntiCheatDetection::Banned;
		}

		if (NewInDetection != FAntiCheatDetection::EAntiCheatDetection::None)
			SetDetection(NewInDetection);
	}

	void KickByAntiCheat(const std::string& PrivateReason, const std::string& KickReason = "[GAC] - Cheat detected (Code DMD)!")
	{
		if (PlayerController)
		{
			AC_LOG(ACLogLevel::ACWarning, "Le joueur [%s] vient de se faire kick par l'Anti-Cheat pour la raison: [%s]", PlayerName.c_str(), PrivateReason.c_str());

			const std::string& Content = "Le joueur: [" + PlayerName + "] vient de se faire kick par l'anti cheat, raison: [" + PrivateReason + "]";
			SendWebhook("Anti Cheat Galaxia", Content);

			Globals::wasWebhookSent = true;

			PlayerController->ClientReturnToMainMenu(std::wstring(KickReason.begin(), KickReason.end()).c_str());

			// [OFFSET-TODO-6.21] NetConnection_Close (8.51 = 0x2cfef80)
			// void (*Close)(UNetConnection* NetConnection) = decltype(Close)(0x2cfef80 + uintptr_t(GetModuleHandle(0)));
			auto Close = Offsets621::Native<void(UNetConnection* NetConnection)>(Offsets621::NetConnection_Close, "NetConnection_Close");
			Close(PlayerController->NetConnection);
		}
	}

	void InitAntiCheatGalaxia(AFortPlayerController* InPC)
	{
		if (!InPC || !InPC->PlayerState)
		{
			if (InPC)
			{
				PlayerController = InPC;
				KickByAntiCheat("Impossible d'init l'Anti Cheat !\nMerci de contacter un developpeur pour fix ce probleme.");
			}

			AC_LOG(ACLogLevel::ACError, "InitAntiCheatGalaxia: Failed to init!");
			return;
		}

		PlayerController = InPC;
		PlayerName = InPC->PlayerState->GetPlayerName().ToString();

		AC_LOG(ACLogLevel::ACLog, "InitAntiCheatGalaxia: Le joueur %s a bien ete init par l'anti-cheat.", PlayerName);
	}
};

static UAntiCheatGalaxia* CreateAntiCheatGalaxia(AFortPlayerController* PlayerController)
{
	UAntiCheatGalaxia* NewClass = new UAntiCheatGalaxia();
	NewClass->InitAntiCheatGalaxia(PlayerController);
	UAntiCheatGalaxia::AntiCheatGalaxiaMap.insert_or_assign(PlayerController, NewClass);
	return NewClass;
}

static UAntiCheatGalaxia* FindOrCreateAntiCheatGalaxia(AFortPlayerController* PlayerController)
{
	UAntiCheatGalaxia* AntiCheatGalaxia = nullptr;

	for (auto& Find : UAntiCheatGalaxia::AntiCheatGalaxiaMap)
	{
		if (Find.first == PlayerController)
		{
			AntiCheatGalaxia = Find.second;
			break;
		}
	}

	if (!AntiCheatGalaxia)
		AntiCheatGalaxia = CreateAntiCheatGalaxia(PlayerController);

	return AntiCheatGalaxia;
}