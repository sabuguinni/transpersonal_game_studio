#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/NoExportTypes.h"
#include "VFX_NiagaraLibrary.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire UMETA(DisplayName = "Campfire"),
    Dust_Impact UMETA(DisplayName = "Dust Impact"),
    Water_Splash UMETA(DisplayName = "Water Splash"),
    Blood_Hit UMETA(DisplayName = "Blood Hit"),
    Steam_Breath UMETA(DisplayName = "Steam Breath"),
    Volcanic_Smoke UMETA(DisplayName = "Volcanic Smoke")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Duration = 5.0f;
        Scale = FVector(1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "VFX", CallInEditor = true)
    static UNiagaraComponent* SpawnVFXAtLocation(
        UObject* WorldContext,
        EVFX_EffectType EffectType,
        FVector Location,
        FRotator Rotation = FRotator::ZeroRotator,
        FVector Scale = FVector(1.0f)
    );

    UFUNCTION(BlueprintCallable, Category = "VFX")
    static UNiagaraComponent* AttachVFXToActor(
        AActor* TargetActor,
        EVFX_EffectType EffectType,
        FName SocketName = NAME_None,
        FVector RelativeLocation = FVector::ZeroVector,
        FRotator RelativeRotation = FRotator::ZeroRotator,
        FVector Scale = FVector(1.0f)
    );

    UFUNCTION(BlueprintCallable, Category = "VFX")
    static void StopVFXEffect(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    static TArray<FVFX_EffectData> GetAvailableEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    static UNiagaraSystem* GetNiagaraSystemForEffect(EVFX_EffectType EffectType);

private:
    static TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> EffectRegistry;
    static void InitializeEffectRegistry();
};