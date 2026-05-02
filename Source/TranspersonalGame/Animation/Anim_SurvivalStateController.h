#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_SurvivalStateController.generated.h"

// Enum para estados de sobrevivência que afetam animação
UENUM(BlueprintType)
enum class EAnim_SurvivalAnimState : uint8
{
    Normal          UMETA(DisplayName = "Normal"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Injured         UMETA(DisplayName = "Injured"),
    Starving        UMETA(DisplayName = "Starving"),
    Dehydrated      UMETA(DisplayName = "Dehydrated"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Gathering       UMETA(DisplayName = "Gathering")
};

// Struct para dados de estado de animação
USTRUCT(BlueprintType)
struct FAnim_SurvivalStateData
{
    GENERATED_BODY()

    // Estado atual de sobrevivência
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    EAnim_SurvivalAnimState CurrentState;

    // Intensidade do estado (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float StateIntensity;

    // Modificador de velocidade baseado no estado
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float SpeedModifier;

    // Modificador de postura (curvatura da coluna)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float PostureModifier;

    // Tremor/shake intensity para estados como medo ou fome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float ShakeIntensity;

    // Tempo no estado atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float TimeInState;

    FAnim_SurvivalStateData()
    {
        CurrentState = EAnim_SurvivalAnimState::Normal;
        StateIntensity = 0.0f;
        SpeedModifier = 1.0f;
        PostureModifier = 0.0f;
        ShakeIntensity = 0.0f;
        TimeInState = 0.0f;
    }
};

/**
 * Controller que gerencia estados de animação baseados em condições de sobrevivência
 * Conecta stats de sobrevivência (fome, sede, medo) com modificadores de animação
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalStateController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalStateController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CONFIGURAÇÃO DE ESTADOS ===

    // Dados atuais do estado de sobrevivência
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    FAnim_SurvivalStateData StateData;

    // Thresholds para diferentes estados
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Thresholds")
    float ExhaustionThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Thresholds")
    float StarvationThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Thresholds")
    float DehydrationThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Thresholds")
    float FearThreshold = 0.4f;

    // Velocidade de transição entre estados
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float StateTransitionSpeed = 2.0f;

    // === MÉTODOS PÚBLICOS ===

    // Atualiza o estado baseado nos stats de sobrevivência
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void UpdateSurvivalState(float Health, float Hunger, float Thirst, float Stamina, float Fear);

    // Define estado manualmente
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void SetSurvivalState(EAnim_SurvivalAnimState NewState, float Intensity = 1.0f);

    // Obtém dados do estado atual
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival Animation")
    FAnim_SurvivalStateData GetStateData() const { return StateData; }

    // Obtém modificador de velocidade atual
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival Animation")
    float GetSpeedModifier() const { return StateData.SpeedModifier; }

    // Obtém modificador de postura atual
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival Animation")
    float GetPostureModifier() const { return StateData.PostureModifier; }

    // Obtém intensidade de tremor atual
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival Animation")
    float GetShakeIntensity() const { return StateData.ShakeIntensity; }

    // Verifica se está num estado específico
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival Animation")
    bool IsInState(EAnim_SurvivalAnimState State) const { return StateData.CurrentState == State; }

protected:
    // === MÉTODOS INTERNOS ===

    // Determina o estado baseado nos stats
    EAnim_SurvivalAnimState DetermineStateFromStats(float Health, float Hunger, float Thirst, float Stamina, float Fear);

    // Calcula modificadores baseados no estado
    void CalculateStateModifiers();

    // Aplica transição suave entre estados
    void ApplyStateTransition(float DeltaTime);

    // === DADOS INTERNOS ===

    // Estado anterior para transições suaves
    EAnim_SurvivalAnimState PreviousState;

    // Valores alvo para interpolação
    float TargetSpeedModifier;
    float TargetPostureModifier;
    float TargetShakeIntensity;

    // Referência ao character owner
    UPROPERTY()
    ACharacter* OwnerCharacter;

    // Referência ao movement component
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;
};