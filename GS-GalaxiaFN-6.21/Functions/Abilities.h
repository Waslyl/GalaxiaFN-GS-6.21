#pragma once

namespace Abilities
{
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// FGameplayAbilitySpecHandle (*GiveAbilityAndActivateOnce)(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle* OutHandle, const FGameplayAbilitySpec& Spec);
	Offsets621::NativeFunc<FGameplayAbilitySpecHandle(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle* OutHandle, const FGameplayAbilitySpec& Spec)> GiveAbilityAndActivateOnce;
	void (*ServerTryActivateAbility)(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& AbilityToActivate, bool InputPressed, const FPredictionKey& PredictionKey);
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// FGameplayAbilitySpecHandle (*GiveAbility)(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle* Handle, FGameplayAbilitySpec Spec);
	Offsets621::NativeFunc<FGameplayAbilitySpecHandle(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle* Handle, FGameplayAbilitySpec Spec)> GiveAbility;
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// char (*InternalTryActivateAbility)(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData);
	Offsets621::NativeFunc<char(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)> InternalTryActivateAbility;
	// [GALAXIA PATCH] NativeFunc : rempli par pattern, l'appel est ignore (avec log) si le pattern n'est pas trouve en 6.21
	// UObject* (*CreateDefaultObject)(UClass* Class);
	Offsets621::NativeFunc<UObject*(UClass* Class)> CreateDefaultObject;

	FGameplayAbilitySpec* FindAbilitySpecFromHandle(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Ability)
	{
		FGameplayAbilitySpec* Spec = nullptr;

		for (int32 i = 0; i < AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
		{
			if (AbilitySystemComponent->ActivatableAbilities.Items[i].Handle.Handle == Ability.Handle)
			{
				return Spec = &AbilitySystemComponent->ActivatableAbilities.Items[i];
			}
		}

		return Spec;
	}

	FGameplayAbilitySpec* FindAbilitySpecFromClass(UAbilitySystemComponent* AbilitySystemComponent, UClass* AbilityClass)
	{
		FGameplayAbilitySpec* Spec = nullptr;

		for (int32 i = 0; i < AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
		{
			if (AbilitySystemComponent->ActivatableAbilities.Items[i].Ability->Class == AbilityClass)
			{
				return Spec = &AbilitySystemComponent->ActivatableAbilities.Items[i];
			}
		}

		return Spec;
	}

	void ApplyGameplayAbility(UClass* AbilityClass, UFortAbilitySystemComponent* AbilitySystemComponent)
	{
		if (!AbilityClass || !AbilitySystemComponent)
			return;

		// [OFFSET-TODO-6.21] CreateDefaultAbilitySpec (8.51 = 0x868290)
		// FGameplayAbilitySpec* (*CreateDefaultAbilitySpec)(FGameplayAbilitySpec* AbilitySpec, UGameplayAbility* Ability, int32 Level, int32 InputID, UObject* SourceObject) = decltype(CreateDefaultAbilitySpec)(0x868290 + uintptr_t(GetModuleHandle(0)));
		auto CreateDefaultAbilitySpec = Offsets621::Native<FGameplayAbilitySpec*(FGameplayAbilitySpec* AbilitySpec, UGameplayAbility* Ability, int32 Level, int32 InputID, UObject* SourceObject)>(Offsets621::CreateDefaultAbilitySpec, "CreateDefaultAbilitySpec");

		// [GALAXIA PATCH] Sans le constructeur natif, le spec garde des TArray non initialises : GiveAbility les copiait et le jeu tombait en "out of memory"
		// FGameplayAbilitySpec AbilitySpec;
		// CreateDefaultAbilitySpec(&AbilitySpec, (UGameplayAbility*)CreateDefaultObject(AbilityClass), 1, -1, nullptr);
		if (!CreateDefaultAbilitySpec || !CreateDefaultObject || !GiveAbility)
			return;

		UGameplayAbility* AbilityDefaultObject = (UGameplayAbility*)CreateDefaultObject(AbilityClass);
		if (!AbilityDefaultObject)
			return;

		FGameplayAbilitySpec AbilitySpec{};
		CreateDefaultAbilitySpec(&AbilitySpec, AbilityDefaultObject, 1, -1, nullptr);

		if (!AbilitySpec.Ability)
			return;

		for (int32 i = 0; i < AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
		{
			auto& CurrentSpec = AbilitySystemComponent->ActivatableAbilities.Items[i];

			if (CurrentSpec.Ability == AbilitySpec.Ability)
				return;
		}

		FGameplayAbilitySpecHandle Handle;
		GiveAbility(AbilitySystemComponent, &Handle, AbilitySpec);
	}

	void GrantGameplayAbility(UFortAbilitySet* AbilitySet, UFortAbilitySystemComponent* AbilitySystemComponent)
	{
		if (!AbilitySet || !AbilitySystemComponent)
			return;

		for (int32 i = 0; i < AbilitySet->GameplayAbilities.Num(); i++)
		{
			TSubclassOf<UFortGameplayAbility> GameplayAbility = AbilitySet->GameplayAbilities[i];

			if (!GameplayAbility.Get())
				continue;

			ApplyGameplayAbility(GameplayAbility, AbilitySystemComponent);
		}
	}

	void GrantGameplayEffect(UFortAbilitySet* AbilitySet, UFortAbilitySystemComponent* AbilitySystemComponent)
	{
		if (!AbilitySet || !AbilitySystemComponent)
			return;

		for (int32 i = 0; i < AbilitySet->GrantedGameplayEffects.Num(); i++)
		{
			FGameplayEffectApplicationInfoHard* GrantedGameplayEffect = &AbilitySet->GrantedGameplayEffects[i];
			if (!GrantedGameplayEffect) continue;

			FGameplayEffectContextHandle EffectContext{};
			AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(GrantedGameplayEffect->GameplayEffect, GrantedGameplayEffect->Level, EffectContext);
		}
	}

	void GrantModifierAbility(UFortAbilitySystemComponent* AbilitySystemComponent, UFortGameplayModifierItemDefinition* ModifierItemDefinition)
	{
		for (int32 i = 0; i < ModifierItemDefinition->PersistentAbilitySets.Num(); i++)
		{
			FFortAbilitySetDeliveryInfo* PersistentAbilitySets = &ModifierItemDefinition->PersistentAbilitySets[i];
			if (!PersistentAbilitySets) continue;

			for (int32 j = 0; j < PersistentAbilitySets->AbilitySets.Num(); j++)
			{
				UFortAbilitySet* AbilitySet = Functions::LoadAbilitySet(PersistentAbilitySets->AbilitySets[j]);

				GrantGameplayAbility(AbilitySet, AbilitySystemComponent);
				GrantGameplayEffect(AbilitySet, AbilitySystemComponent);
			}
		}
	}

	void GrantModifierAbilityFromPlaylist(UFortAbilitySystemComponent* AbilitySystemComponent)
	{
		UFortPlaylistAthena* PlaylistAthena = Globals::GetPlaylist();

		// Todo: à fix cette merde
		if (PlaylistAthena && false)
		{
			for (int32 i = 0; i < PlaylistAthena->ModifierList.Num(); i++)
			{
				UFortGameplayModifierItemDefinition* ModifierItemDefinition = Functions::LoadGameplayModifier(PlaylistAthena->ModifierList[i]);

				if (!ModifierItemDefinition)
					continue;

				GrantModifierAbility(AbilitySystemComponent, ModifierItemDefinition);
			}
		}
	}

	void InternalServerTryActiveAbilityHook(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData)
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilitySystemComponent, Handle);

		if (!Spec)
		{
			AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
			return;
		}

		UGameplayAbility* InstancedAbility = nullptr;
		Spec->InputPressed = true;

		if (!InternalTryActivateAbility(AbilitySystemComponent, Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
		{
			AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
			Spec->InputPressed = false;
		}

		AbilitySystemComponent->ActivatableAbilities.MarkItemDirty(*Spec);
	}

	void InitAbilities()
	{
		static auto FortAbilitySystemComponentAthenaDefault = FindObjectFast<UObject>("/Script/FortniteGame.Default__FortAbilitySystemComponentAthena");

		uintptr_t AddressGiveAbility = MinHook::FindPatternUnique(Patterns::GiveAbility, "GiveAbility");
		uintptr_t AddressInternalTryActivateAbility = MinHook::FindPatternUnique(Patterns::InternalTryActivateAbility, "InternalTryActivateAbility");
		uintptr_t AddressCreateDefaultObject = MinHook::FindPatternUnique(Patterns::CreateDefaultObject, "CreateDefaultObject");
		uintptr_t AddressGiveAbilityAndActivateOnce = MinHook::FindPatternUnique(Patterns::GiveAbilityAndActivateOnce, "GiveAbilityAndActivateOnce");

		GiveAbility.Func = decltype(GiveAbility.Func)(AddressGiveAbility);
		GiveAbility.Name = "GiveAbility";
		InternalTryActivateAbility.Func = decltype(InternalTryActivateAbility.Func)(AddressInternalTryActivateAbility);
		InternalTryActivateAbility.Name = "InternalTryActivateAbility";
		CreateDefaultObject.Func = decltype(CreateDefaultObject.Func)(AddressCreateDefaultObject);
		CreateDefaultObject.Name = "CreateDefaultObject";
		GiveAbilityAndActivateOnce.Func = decltype(GiveAbilityAndActivateOnce.Func)(AddressGiveAbilityAndActivateOnce);
		GiveAbilityAndActivateOnce.Name = "GiveAbilityAndActivateOnce";

		// [8.51-ONLY] Index 8.51
		// MinHook::HookVTable(FortAbilitySystemComponentAthenaDefault, 0xF4, InternalServerTryActiveAbilityHook, nullptr, "InternalServerTryActiveAbility");
		// [GALAXIA PATCH] 6.21 : slot 0xF3 verifie dans le dump (ServerTryActivateAbility 0x6666E0 appelle vtable+0x798). Tous les chemins y passent, y compris ServerAbilityRPCBatch, utilise pour le tir des armes
		MinHook::HookVTable(FortAbilitySystemComponentAthenaDefault, 0xF3, InternalServerTryActiveAbilityHook, nullptr, "InternalServerTryActiveAbility");

		FN_LOG(LogInit, Log, "InitAbilities Success!");
	}
}