#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../Core/ConsciousnessSystem.h"
#include "ConsciousnessAwareCombat.generated.h"

UENUM(BlueprintType)
enum class ECombatConsciousnessResponse : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Transformative  UMETA(DisplayName = "Transformative"),
    Transcendent    UMETA(DisplayName = "Transcendent")
};

USTRUCT(BlueprintType)
struct FConsciousnessCombatModifiers
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PerceptionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeRedeemed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUsesEnergyAttacks = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessAwareCombat : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessAwareCombat();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Combat")
    TMap<EConsciousnessState, FConsciousnessCombatModifiers> ConsciousnessModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Combat")
    ECombatConsciousnessResponse CurrentResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Combat")
    float ConsciousnessInfluenceRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Combat")
    bool bCanTransformEnemies = false;

    UPROPERTY(BlueprintReadOnly, Category = "Consciousness Combat")
    class UConsciousnessSystem* ConsciousnessSystem;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Consciousness Combat")
    void UpdateCombatResponse(EConsciousnessState PlayerState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Combat")
    FConsciousnessCombatModifiers GetCurrentModifiers() const;

    UFUNCTION(BlueprintCallable, Category = "Consciousness Combat")
    bool ShouldEngageInCombat(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Consciousness Combat")
    float CalculateDamageModifier(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Consciousness Combat")
    bool AttemptEnemyTransformation(AActor* Enemy);

    UFUNCTION(BlueprintImplementableEvent, Category = "Consciousness Combat")
    void OnCombatResponseChanged(ECombatConsciousnessResponse NewResponse);

    UFUNCTION(BlueprintImplementableEvent, Category = "Consciousness Combat")
    void OnEnemyTransformed(AActor* TransformedEnemy);

private:
    void InitializeConsciousnessModifiers();
    ECombatConsciousnessResponse DetermineResponseFromState(EConsciousnessState State);
};