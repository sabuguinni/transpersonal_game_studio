// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/TriggerVolume.h"
#include "ConsciousnessZone.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessZoneType : uint8
{
    Meditation      UMETA(DisplayName = "Meditation Zone"),
    Transformation  UMETA(DisplayName = "Transformation Zone"),
    Challenge       UMETA(DisplayName = "Challenge Zone"),
    Sanctuary       UMETA(DisplayName = "Sanctuary Zone"),
    Vortex          UMETA(DisplayName = "Energy Vortex"),
    Portal          UMETA(DisplayName = "Dimensional Portal")
};

USTRUCT(BlueprintType)
struct FConsciousnessZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessZoneType ZoneType = EConsciousnessZoneType::Meditation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RequiredConsciousnessLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float ConsciousnessInfluenceRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float InfluenceStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableVisualEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableAudioEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableParticleEffects = true;

    FConsciousnessZoneConfig()
    {
        ZoneType = EConsciousnessZoneType::Meditation;
        RequiredConsciousnessLevel = 0.0f;
        ConsciousnessInfluenceRadius = 500.0f;
        InfluenceStrength = 1.0f;
        bEnableVisualEffects = true;
        bEnableAudioEffects = true;
        bEnableParticleEffects = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnterZone, class ATranspersonalCharacter*, Player, EConsciousnessZoneType, ZoneType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerExitZone, class ATranspersonalCharacter*, Player, EConsciousnessZoneType, ZoneType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnConsciousnessThresholdReached, class ATranspersonalCharacter*, Player, float, ConsciousnessLevel, EConsciousnessZoneType, ZoneType);

/**
 * Consciousness Zone - Areas that respond to and influence player consciousness levels
 * These zones can enhance meditation, trigger transformations, or create challenges
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AConsciousnessZone : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessZone();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* ZoneCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ZoneVisualization;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* ZoneAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* ZoneParticles;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Zone")
    FConsciousnessZoneConfig ZoneConfig;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerEnterZone OnPlayerEnterZone;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerExitZone OnPlayerExitZone;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnConsciousnessThresholdReached OnConsciousnessThresholdReached;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness Zone")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Consciousness Zone")
    void DeactivateZone();

    UFUNCTION(BlueprintCallable, Category = "Consciousness Zone")
    bool IsPlayerInZone(class ATranspersonalCharacter* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Consciousness Zone")
    float GetConsciousnessInfluenceForPlayer(class ATranspersonalCharacter* Player) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Effects")
    void UpdateZoneEffects(float ConsciousnessLevel, float DeltaTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "Effects")
    void OnZoneActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Effects")
    void OnZoneDeactivated();

protected:
    UFUNCTION()
    void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                           bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void UpdateZoneForPlayer(class ATranspersonalCharacter* Player, float DeltaTime);
    void ApplyConsciousnessInfluence(class ATranspersonalCharacter* Player, float DeltaTime);

private:
    UPROPERTY()
    TArray<class ATranspersonalCharacter*> PlayersInZone;

    UPROPERTY()
    bool bIsZoneActive;

    UPROPERTY()
    float ZoneIntensity;

    UPROPERTY()
    float LastConsciousnessLevel;
};