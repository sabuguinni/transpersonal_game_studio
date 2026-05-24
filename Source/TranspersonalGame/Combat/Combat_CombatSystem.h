#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerSphere.h"
#include "../SharedTypes.h"
#include "Combat_CombatSystem.generated.h"

class ADinosaurAIController;
class ATranspersonalCharacter;

/**
 * Sistema central de combate que gere encontros entre jogador e dinossauros
 * Implementa detecção de proximidade, cálculo de dano e coordenação de IA
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_CombatSystem : public AActor
{
    GENERATED_BODY()

public:
    ACombat_CombatSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componente de detecção de combate
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    USphereComponent* CombatDetectionSphere;

    // Raio de detecção de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatRadius;

    // Dano base por proximidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ProximityDamage;

    // Intervalo de dano (segundos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DamageInterval;

    // Multiplicador de dano por tipo de dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TMap<ENPC_DinosaurSpecies, float> DamageMultipliers;

    // Estado atual do combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombat_CombatState CurrentCombatState;

    // Dinossauros atualmente em combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> ActiveCombatants;

    // Jogador atual
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ATranspersonalCharacter* PlayerCharacter;

    // Timer para dano contínuo
    FTimerHandle DamageTimerHandle;

public:
    // Funções de combate
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCombat(AActor* Instigator, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddCombatant(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemoveCombatant(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsActorInCombat(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateDamage(AActor* Attacker, AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamageToPlayer(float Damage, AActor* DamageSource);

    // Funções de detecção
    UFUNCTION()
    void OnCombatSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnCombatSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Sistema de coordenação de IA
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack(const TArray<AActor*>& PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetDinosaurCombatBehavior(AActor* Dinosaur, ECombat_DinosaurCombatBehavior Behavior);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetNearbyDinosaurs(float SearchRadius) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldDinosaurAttack(AActor* Dinosaur, AActor* Target) const;

protected:
    // Funções internas
    void InitializeCombatSystem();
    void UpdateCombatState();
    void ProcessProximityDamage();
    void HandlePackCoordination();
    
    // Configuração por espécie
    void SetupDamageMultipliers();
    
    // Validação
    bool IsValidCombatant(AActor* Actor) const;
    ENPC_DinosaurSpecies GetDinosaurSpecies(AActor* Dinosaur) const;
    
    // Eventos
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnCombatStarted(AActor* Instigator, AActor* Target);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnCombatEnded();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnPlayerDamaged(float Damage, AActor* Source);
};