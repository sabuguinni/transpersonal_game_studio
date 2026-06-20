// BiomeAudioSystem.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_003
// Biome-aware ambient audio trigger system for the prehistoric survival world.
// Each biome zone has distinct ambient soundscape cues tied to geography.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "BiomeAudioSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeAudioType : uint8
{
    Forest      UMETA(DisplayName = "Dense Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    River       UMETA(DisplayName = "River Bank"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Volcanic    UMETA(DisplayName = "Volcanic Region"),
    Cave        UMETA(DisplayName = "Cave Interior"),
    COUNT       UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FWorld_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    EWorld_BiomeAudioType BiomeType = EWorld_BiomeAudioType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float TriggerRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float FadeInTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float FadeOutTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    bool bPlayDinoCallsInZone = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float DinoCallInterval = 30.0f;
};

UCLASS(ClassGroup = (World), meta = (DisplayName = "Biome Audio Zone"))
class TRANSPERSONALGAME_API ABiomeAudioZone : public AActor
{
    GENERATED_BODY()

public:
    ABiomeAudioZone();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BiomeAudio",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BiomeAudio",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    FWorld_BiomeAudioConfig BiomeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    bool bIsPlayerInZone = false;

    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    void OnPlayerEnterZone(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    void OnPlayerExitZone(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    void FadeInAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    void FadeOutAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    EWorld_BiomeAudioType GetBiomeType() const { return BiomeConfig.BiomeType; }

    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    float GetTriggerRadius() const { return BiomeConfig.TriggerRadius; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float DinoCallTimer = 0.0f;
    float CurrentFadeProgress = 0.0f;
    bool bFadingIn = false;
    bool bFadingOut = false;

    void TickDinoCallTimer(float DeltaTime);
    void TickAudioFade(float DeltaTime);
    FString GetBiomeDebugName() const;
};
