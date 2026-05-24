#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../AI/NPC_EmotionalBehaviorSystem.h"
#include "Combat_EmotionalIntegrationSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_EmotionalCombatState : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Defensive   UMETA(DisplayName = "Defensive"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Protective  UMETA(DisplayName = "Protective"),
    Enraged     UMETA(DisplayName = "Enraged")
};

USTRUCT(BlueprintType)
struct FCombat_EmotionalCombatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    ECombat_EmotionalCombatState CombatState = ECombat_EmotionalCombatState::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    float ProtectiveInstinct = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    float CombatEffectiveness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    float LastCombatTime = 0.0f;

    FCombat_EmotionalCombatData()
    {
        CombatState = ECombat_EmotionalCombatState::Neutral;
        AggressionLevel = 0.5f;
        FearLevel = 0.0f;
        ProtectiveInstinct = 0.3f;
        CombatEffectiveness = 1.0f;
        bIsInCombat = false;
        CurrentTarget = nullptr;
        LastCombatTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_EmotionalIntegrationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_EmotionalIntegrationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Sistema de integração emocional
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    FCombat_EmotionalCombatData CombatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    float EmotionalUpdateRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    float CombatDetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Combat")
    float ThreatAssessmentRange = 2000.0f;

    // Referência ao sistema emocional dos NPCs
    UPROPERTY(BlueprintReadOnly, Category = "Emotional Combat")
    class UNPC_EmotionalBehaviorSystem* EmotionalSystem;

    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    void UpdateEmotionalCombatState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    void ProcessEmotionalTriggers();

    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    void HandleCombatStateTransition(ECombat_EmotionalCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    void AssessThreatLevel(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    void ModifyAggressionBasedOnEmotion(float EmotionalIntensity);

    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    void HandleProtectiveResponse(AActor* ThreatenedAlly);

    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    void ProcessFearResponse(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    bool ShouldEngageInCombat(AActor* PotentialTarget);

    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    float CalculateCombatEffectiveness();

    UFUNCTION(BlueprintCallable, Category = "Emotional Combat")
    void ApplyEmotionalCombatModifiers();

private:
    float LastEmotionalUpdate;
    TArray<AActor*> NearbyThreats;
    TArray<AActor*> NearbyAllies;

    void ScanForThreatsAndAllies();
    void UpdateCombatMetrics(float DeltaTime);
    void ProcessEmotionalInfluence(float DeltaTime);
};