#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "CombatAIManager.generated.h"

class ADinosaurCharacter;
class UCombatBehaviorComponent;
class UThreatAssessmentComponent;

/**
 * Sistema central de gerenciamento de IA de combate
 * Coordena comportamentos táticos entre múltiplos dinossauros
 * Implementa sistema de ameaças dinâmico e comunicação entre IAs
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombatAIManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Sistema de Registro de Dinossauros
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<ADinosaurCharacter*> ActiveDinosaurs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<ADinosaurCharacter*> CombatActiveDinosaurs;

    // Sistema de Ameaças Globais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Threat System")
    TMap<AActor*, float> GlobalThreatMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat System")
    float ThreatDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat System")
    float MaxThreatDistance = 2000.0f;

public:
    // Registro e Gestão de Dinossauros
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterDinosaur(ADinosaurCharacter* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterDinosaur(ADinosaurCharacter* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EnterCombatMode(ADinosaurCharacter* Dinosaur, AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExitCombatMode(ADinosaurCharacter* Dinosaur);

    // Sistema de Ameaças
    UFUNCTION(BlueprintCallable, Category = "Threat System")
    void UpdateThreatLevel(AActor* ThreatActor, float ThreatValue, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Threat System")
    float GetThreatLevel(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "Threat System")
    TArray<AActor*> GetNearbyThreats(FVector Location, float Radius) const;

    // Comunicação entre IAs
    UFUNCTION(BlueprintCallable, Category = "AI Communication")
    void BroadcastAlert(FVector AlertLocation, AActor* ThreatActor, float AlertRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "AI Communication")
    void BroadcastPackHunt(ADinosaurCharacter* PackLeader, AActor* Target);

    // Sistema de Coordenação Tática
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector GetFlankingPosition(ADinosaurCharacter* Attacker, AActor* Target, float PreferredDistance = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldRetreat(ADinosaurCharacter* Dinosaur, AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    ADinosaurCharacter* GetNearestAlly(ADinosaurCharacter* Dinosaur, float MaxDistance = 1500.0f);

protected:
    // Métodos internos de gestão
    void UpdateThreatDecay(float DeltaTime);
    void ProcessCombatCoordination();
    void UpdatePackBehaviors();

    // Cache para otimização
    float LastThreatUpdateTime;
    float ThreatUpdateInterval = 0.5f;
};