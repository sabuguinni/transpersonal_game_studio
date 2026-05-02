#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_SurvivalStateManager.generated.h"

// Estados de sobrevivência para animação
UENUM(BlueprintType)
enum class EAnim_SurvivalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Injured     UMETA(DisplayName = "Injured"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Hiding      UMETA(DisplayName = "Hiding"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

// Tipos de terreno que afectam animação
UENUM(BlueprintType)
enum class EAnim_TerrainType : uint8
{
    Flat        UMETA(DisplayName = "Flat"),
    Rocky       UMETA(DisplayName = "Rocky"),
    Muddy       UMETA(DisplayName = "Muddy"),
    Sandy       UMETA(DisplayName = "Sandy"),
    Steep       UMETA(DisplayName = "Steep"),
    Uneven      UMETA(DisplayName = "Uneven"),
    Slippery    UMETA(DisplayName = "Slippery"),
    Water       UMETA(DisplayName = "Water"),
    Snow        UMETA(DisplayName = "Snow")
};

// Dados de estado para animação
USTRUCT(BlueprintType)
struct FAnim_SurvivalData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    bool bIsFalling = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
    bool bIsJumping = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float HealthPercentage = 100.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float StaminaPercentage = 100.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float HungerLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Survival")
    float ThirstLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
    EAnim_SurvivalState CurrentState = EAnim_SurvivalState::Idle;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    EAnim_TerrainType TerrainType = EAnim_TerrainType::Flat;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    float TerrainSlope = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    bool bIsInDanger = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environment")
    bool bIsHidden = false;
};

/**
 * Componente que gere estados de animação baseados em sobrevivência
 * Integra com sistemas de saúde, stamina, medo e ambiente
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalStateManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalStateManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === INTERFACE PÚBLICA ===

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    FAnim_SurvivalData GetCurrentAnimationData() const { return CurrentAnimData; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    EAnim_SurvivalState GetCurrentState() const { return CurrentAnimData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetSurvivalState(EAnim_SurvivalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void UpdateSurvivalStats(float Health, float Stamina, float Fear, float Hunger, float Thirst);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetTerrainInfo(EAnim_TerrainType Terrain, float Slope);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetDangerState(bool bInDanger);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    bool ShouldPlayInjuredAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    bool ShouldPlayExhaustedAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    bool ShouldPlayFearfulAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    float GetMovementSpeedModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    float GetAnimationPlayRate() const;

protected:
    // === DADOS INTERNOS ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    FAnim_SurvivalData CurrentAnimData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float StateTransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float InjuredThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ExhaustedThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float FearThreshold = 70.0f;

    // === MÉTODOS INTERNOS ===

    void UpdateMovementData();
    void UpdateSurvivalStateLogic();
    void AnalyzeTerrainConditions();
    EAnim_SurvivalState DetermineOptimalState() const;
    void SmoothStateTransition(float DeltaTime);

    // Referências para componentes do personagem
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    // Timers para transições
    float StateTransitionTimer = 0.0f;
    EAnim_SurvivalState PreviousState = EAnim_SurvivalState::Idle;
    EAnim_SurvivalState TargetState = EAnim_SurvivalState::Idle;
};