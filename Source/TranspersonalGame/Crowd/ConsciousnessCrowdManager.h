#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "../Core/ConsciousnessStateComponent.h"
#include "ConsciousnessCrowdManager.generated.h"

UENUM(BlueprintType)
enum class ECrowdBehaviorState : uint8
{
    Calm,
    Anxious,
    Panicked,
    Enlightened,
    Synchronized
};

USTRUCT(BlueprintType)
struct FCrowdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessResonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StressLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AwarenessRadius;

    FCrowdMember()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        ConsciousnessResonance = 0.5f;
        BehaviorState = ECrowdBehaviorState::Calm;
        StressLevel = 0.0f;
        AwarenessRadius = 200.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AConsciousnessCrowdManager : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessCrowdManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core crowd properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float ConsciousnessInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float ConsciousnessInfluenceStrength;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MaxForce;

    // Consciousness response curves
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Response")
    class UCurveFloat* CalmnessBehaviorCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Response")
    class UCurveFloat* AwarenessBehaviorCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Response")
    class UCurveFloat* EnergyBehaviorCurve;

    // Crowd members
    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    TArray<FCrowdMember> CrowdMembers;

    // Player consciousness reference
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UConsciousnessStateComponent* PlayerConsciousness;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdMember(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void RemoveCrowdMember(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateConsciousnessInfluence();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    ECrowdBehaviorState GetDominantBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    float GetCrowdHarmonyLevel() const;

protected:
    // Core simulation functions
    void UpdateCrowdBehavior(float DeltaTime);
    void ApplyFlockingBehavior(FCrowdMember& Member, float DeltaTime);
    void ApplyConsciousnessInfluence(FCrowdMember& Member, float DeltaTime);
    void UpdateBehaviorState(FCrowdMember& Member);

    // Flocking algorithms
    FVector CalculateSeparation(const FCrowdMember& Member);
    FVector CalculateAlignment(const FCrowdMember& Member);
    FVector CalculateCohesion(const FCrowdMember& Member);
    FVector CalculateAvoidance(const FCrowdMember& Member);

    // Consciousness-based behaviors
    FVector CalculateConsciousnessAttraction(const FCrowdMember& Member);
    FVector CalculateResonanceField(const FCrowdMember& Member);
    void PropagateConsciousnessWave(const FVector& Origin, float Intensity);

    // Utility functions
    TArray<int32> GetNearbyMembers(const FCrowdMember& Member, float Radius);
    float CalculateConsciousnessDistance(const FCrowdMember& Member);
    void ClampVelocity(FCrowdMember& Member);

private:
    // Internal state
    float LastConsciousnessUpdate;
    float ConsciousnessUpdateInterval;
    TArray<FVector> ConsciousnessWaveOrigins;
    TArray<float> ConsciousnessWaveIntensities;
    TArray<float> ConsciousnessWaveTimes;
};