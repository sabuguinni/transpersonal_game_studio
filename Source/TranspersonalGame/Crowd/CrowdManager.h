// CrowdManager.h
// Sistema de gestão de multidões conscientes que respondem ao estado espiritual do jogador

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../Core/ConsciousnessSystem.h"
#include "CrowdManager.generated.h"

UENUM(BlueprintType)
enum class ECrowdBehaviorState : uint8
{
    Peaceful,      // Estado pacífico - multidão calma
    Agitated,      // Estado agitado - multidão inquieta
    Hostile,       // Estado hostil - multidão agressiva
    Transcendent,  // Estado transcendente - multidão em harmonia
    Fearful        // Estado de medo - multidão em pânico
};

USTRUCT(BlueprintType)
struct FCrowdUnit
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InfluenceRadius;

    FCrowdUnit()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        ConsciousnessLevel = 0.5f;
        BehaviorState = ECrowdBehaviorState::Peaceful;
        InfluenceRadius = 200.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Configurações da multidão
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float ConsciousnessInfluenceRange;

    // Array de unidades da multidão
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd State")
    TArray<FCrowdUnit> CrowdUnits;

    // Estado atual da multidão
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd State")
    ECrowdBehaviorState CurrentCrowdState;

    // Referência ao sistema de consciência
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    class UConsciousnessComponent* ConsciousnessSystem;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdUnits(int32 Count, FVector CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdBehavior(float PlayerConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdState(ECrowdBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void ClearCrowd();

    // Eventos Blueprint
    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd Events")
    void OnCrowdStateChanged(ECrowdBehaviorState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd Events")
    void OnCrowdUnitSpawned(const FCrowdUnit& NewUnit);

private:
    // Funções internas
    void UpdateCrowdMovement(float DeltaTime);
    void ApplyConsciousnessInfluence();
    void CalculateFlocking(FCrowdUnit& Unit, const TArray<FCrowdUnit>& NearbyUnits);
    FVector CalculateSeparation(const FCrowdUnit& Unit, const TArray<FCrowdUnit>& NearbyUnits);
    FVector CalculateAlignment(const FCrowdUnit& Unit, const TArray<FCrowdUnit>& NearbyUnits);
    FVector CalculateCohesion(const FCrowdUnit& Unit, const TArray<FCrowdUnit>& NearbyUnits);
    TArray<FCrowdUnit> GetNearbyUnits(const FCrowdUnit& Unit, float Radius);

    // Configurações de flocking
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, Category = "Flocking")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, Category = "Flocking")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, Category = "Flocking")
    float FlockingRadius;

    // Timer para otimização
    float LastUpdateTime;
    float UpdateInterval;
};