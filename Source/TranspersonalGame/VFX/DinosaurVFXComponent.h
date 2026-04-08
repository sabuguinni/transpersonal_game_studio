#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "VFXManager.h"
#include "DinosaurVFXComponent.generated.h"

UENUM(BlueprintType)
enum class EDinosaurVFXState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Alert           UMETA(DisplayName = "Alert"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Domesticated    UMETA(DisplayName = "Domesticated"),
    Injured         UMETA(DisplayName = "Injured"),
    Dead            UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FDinosaurVFXProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Size = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor::Gray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BreathingIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FootstepIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UniqueEffects;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurVFXComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurVFXComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // State Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void SetVFXState(EDinosaurVFXState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    EDinosaurVFXState GetCurrentVFXState() const { return CurrentVFXState; }

    // Profile Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void SetDinosaurProfile(const FDinosaurVFXProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    FDinosaurVFXProfile GetDinosaurProfile() const { return DinosaurProfile; }

    // Breathing Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void StartBreathingEffect();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void StopBreathingEffect();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void UpdateBreathingIntensity(float NewIntensity);

    // Movement Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerFootstepEffect(FVector FootLocation, bool bIsRightFoot = true);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void StartMovementTrail();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void StopMovementTrail();

    // Emotional State Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void ShowAlertEffect(float AlertLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void ShowAggressionEffect(float AggressionLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void ShowDomesticationEffect(float TrustLevel = 0.0f);

    // Health Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void ShowInjuryEffect(FVector InjuryLocation, float Severity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void StartBleedingEffect(FVector BleedLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void StopBleedingEffect();

    // Unique Species Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerSpeciesUniqueEffect(FName EffectName, float Intensity = 1.0f);

    // Environmental Interaction
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void ShowEnvironmentInteraction(FName InteractionType, FVector Location);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Profile")
    FDinosaurVFXProfile DinosaurProfile;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    EDinosaurVFXState CurrentVFXState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    EDinosaurVFXState PreviousVFXState;

    // Active VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active VFX")
    UNiagaraComponent* BreathingVFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active VFX")
    UNiagaraComponent* MovementTrailVFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active VFX")
    UNiagaraComponent* EmotionalStateVFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active VFX")
    UNiagaraComponent* HealthStatusVFX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active VFX")
    TArray<UNiagaraComponent*> UniqueEffectsVFX;

    // VFX Manager Reference
    UPROPERTY()
    UVFXManager* VFXManager;

    // Timing and State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float BreathingRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float FootstepCooldown = 0.5f;

    UPROPERTY()
    float LastFootstepTime;

    UPROPERTY()
    float LastBreathTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoUpdateBreathing = true;

private:
    void InitializeVFXManager();
    void UpdateStateTransitionEffects();
    void CleanupVFXComponents();
    FVector GetBreathingLocation() const;
    FName GetStateSpecificEffect(EDinosaurVFXState State) const;
};