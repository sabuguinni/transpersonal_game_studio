#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "VFXManager.h"
#include "DinosaurVFXComponent.generated.h"

UENUM(BlueprintType)
enum class EDinosaurVFXState : uint8
{
    Idle,
    Moving,
    Attacking,
    Roaring,
    Injured,
    Dying,
    Feeding
};

USTRUCT(BlueprintType)
struct FDinosaurVFXSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableBreathVFX = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableFootstepVFX = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableBloodVFX = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableRoarVFX = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VFXIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DinosaurColorTint = FLinearColor::White;
};

/**
 * Dinosaur VFX Component - Manages all visual effects specific to dinosaur creatures
 * Handles breath effects, footsteps, combat VFX, and behavioral visual cues
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurVFXComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurVFXComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void SetDinosaurVFXState(EDinosaurVFXState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    EDinosaurVFXState GetCurrentVFXState() const { return CurrentVFXState; }

    // Breath Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Breath")
    void StartBreathEffect(const FString& BreathType = "Normal");

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Breath")
    void StopBreathEffect();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Breath")
    void TriggerRoarBreath(float Duration = 3.0f);

    // Movement Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Movement")
    void TriggerFootstepEffect(const FVector& FootLocation, const FString& SurfaceType = "Ground");

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Movement")
    void StartMovementTrail();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Movement")
    void StopMovementTrail();

    // Combat Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Combat")
    void TriggerAttackEffect(const FVector& AttackLocation, const FVector& AttackDirection);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Combat")
    void TriggerInjuryEffect(const FVector& InjuryLocation, float Severity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Combat")
    void StartBleedingEffect(const FVector& BleedLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Combat")
    void StopBleedingEffect();

    // Behavioral Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Behavior")
    void TriggerRoarEffect(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Behavior")
    void StartFeedingEffect();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Behavior")
    void StopFeedingEffect();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Behavior")
    void TriggerTerritorialDisplay();

    // Environmental Interaction
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Environment")
    void TriggerWaterSplash(const FVector& WaterLocation, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Environment")
    void StartDustCloud();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Environment")
    void StopDustCloud();

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Settings")
    void UpdateVFXSettings(const FDinosaurVFXSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Settings")
    void SetDinosaurSize(float SizeMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX|Settings")
    void SetDinosaurType(const FString& DinosaurType);

protected:
    // VFX Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FDinosaurVFXSettings VFXSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FString DinosaurType = "Generic";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float DinosaurSizeMultiplier = 1.0f;

    // Component References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    FName MouthSocketName = "MouthSocket";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    FName LeftFootSocketName = "LeftFootSocket";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    FName RightFootSocketName = "RightFootSocket";

    // Active VFX Components
    UPROPERTY()
    UNiagaraComponent* BreathVFXComponent;

    UPROPERTY()
    UNiagaraComponent* MovementTrailComponent;

    UPROPERTY()
    UNiagaraComponent* BleedingVFXComponent;

    UPROPERTY()
    UNiagaraComponent* DustCloudComponent;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EDinosaurVFXState CurrentVFXState = EDinosaurVFXState::Idle;

    UPROPERTY()
    UVFXManager* VFXManager;

private:
    // Internal Functions
    void InitializeVFXComponents();
    void CleanupVFXComponents();
    UNiagaraComponent* GetOrCreateVFXComponent(EVFXType VFXType, const FName& AttachSocket = NAME_None);
    void UpdateVFXBasedOnState();
    FVector GetSocketLocation(const FName& SocketName) const;

    // Timer Handles
    FTimerHandle BreathEffectTimer;
    FTimerHandle BleedingEffectTimer;
};