#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental,      // Ambient effects - fog, dust, particles
    Combat,            // Impact, blood, destruction
    Creature,          // Dinosaur-specific effects
    Survival,          // Fire, smoke, crafting sparks
    Atmospheric,       // Weather, lighting effects
    Interaction,       // Domestication progress, fear indicators
    Destruction        // Breaking objects, falling debris
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle,           // Background ambient effects
    Medium,           // Noticeable but not distracting
    High,             // Dramatic moments, combat
    Critical          // Life/death situations, major events
};

USTRUCT(BlueprintType)
struct FVFXSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAttachToActor;

    FVFXSettings()
    {
        Category = EVFXCategory::Environmental;
        Intensity = EVFXIntensity::Medium;
        Duration = 5.0f;
        bLooping = false;
        FadeInTime = 0.5f;
        FadeOutTime = 1.0f;
        bAttachToActor = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* PlayVFXAtLocation(
        class UNiagaraSystem* VFXSystem,
        const FVector& Location,
        const FRotator& Rotation = FRotator::ZeroRotator,
        const FVFXSettings& Settings = FVFXSettings()
    );

    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* PlayVFXAttached(
        class UNiagaraSystem* VFXSystem,
        class USceneComponent* AttachComponent,
        const FName& AttachPointName = NAME_None,
        const FVector& Location = FVector::ZeroVector,
        const FRotator& Rotation = FRotator::ZeroRotator,
        const FVFXSettings& Settings = FVFXSettings()
    );

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnAmbientFog(const FVector& Location, float Radius, float Density);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnDustParticles(const FVector& Location, const FVector& WindDirection, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnLeafFall(const FVector& Location, float Radius);

    // Creature VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Creature")
    void SpawnFootstepDust(const FVector& Location, float CreatureSize);

    UFUNCTION(BlueprintCallable, Category = "VFX|Creature")
    void SpawnBreathSteam(class USceneComponent* AttachPoint, float Temperature);

    UFUNCTION(BlueprintCallable, Category = "VFX|Creature")
    void SpawnTerritoryMarker(const FVector& Location, class AActor* OwnerCreature);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnImpactEffect(const FVector& Location, const FVector& Normal, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodSpray(const FVector& Location, const FVector& Direction, float Amount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnClawScratch(const FVector& StartLocation, const FVector& EndLocation);

    // Survival VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void SpawnCampfire(const FVector& Location, float Size);

    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void SpawnCraftingSparks(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void SpawnCookingSmoke(const FVector& Location);

    // Interaction VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Interaction")
    void SpawnDomesticationProgress(class AActor* Creature, float TrustLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Interaction")
    void SpawnFearIndicator(class AActor* Creature, float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Interaction")
    void SpawnCalmingAura(const FVector& Location, float Radius);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetVFXQualityLevel(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void EnableLODSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetMaxActiveVFX(int32 MaxCount);

protected:
    // VFX Assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Environmental")
    class UNiagaraSystem* NS_AmbientFog;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Environmental")
    class UNiagaraSystem* NS_DustParticles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Environmental")
    class UNiagaraSystem* NS_LeafFall;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Creature")
    class UNiagaraSystem* NS_FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Creature")
    class UNiagaraSystem* NS_BreathSteam;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Creature")
    class UNiagaraSystem* NS_TerritoryMarker;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Combat")
    class UNiagaraSystem* NS_ImpactEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Combat")
    class UNiagaraSystem* NS_BloodSpray;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Combat")
    class UNiagaraSystem* NS_ClawScratch;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Survival")
    class UNiagaraSystem* NS_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Survival")
    class UNiagaraSystem* NS_CraftingSparks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Survival")
    class UNiagaraSystem* NS_CookingSmoke;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Interaction")
    class UNiagaraSystem* NS_DomesticationProgress;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Interaction")
    class UNiagaraSystem* NS_FearIndicator;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Assets|Interaction")
    class UNiagaraSystem* NS_CalmingAura;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxActiveVFXCount;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bUseLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CurrentQualityLevel;

    // Active VFX Tracking
    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveVFXComponents;

private:
    void CleanupFinishedVFX();
    void ApplyLODSettings(class UNiagaraComponent* VFXComponent, float Distance);
    bool CanSpawnNewVFX() const;
};