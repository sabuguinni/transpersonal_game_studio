#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Player          UMETA(DisplayName = "Player"),
    Combat          UMETA(DisplayName = "Combat"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Atmosphere      UMETA(DisplayName = "Atmosphere")
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle          UMETA(DisplayName = "Subtle"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAttachToActor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AttachSocket = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;

    FVFXDefinition()
    {
        EffectName = NAME_None;
        Category = EVFXCategory::Environment;
        Intensity = EVFXIntensity::Medium;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* PlayVFX(FName EffectName, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXByCategory(EVFXCategory Category);

    // Dinosaur-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayDinosaurBreathingEffect(AActor* DinosaurActor, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayDinosaurFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayDomesticationProgressEffect(AActor* DinosaurActor, float TrustLevel);

    // Environment VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void PlayAmbientForestEffect(FVector Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void PlayDangerIndicatorEffect(FVector Location, float ThreatLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void PlayWeatherEffect(FName WeatherType, float Intensity = 1.0f);

    // Player VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Player")
    void PlayCraftingEffect(FVector Location, FName ItemType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Player")
    void PlayGatheringEffect(FVector Location, FName ResourceType);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetVFXQualityLevel(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void UpdateLODDistances();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Database")
    TArray<FVFXDefinition> VFXDefinitions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveVFX = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalVFXIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVFXCulling = true;

private:
    void CleanupInactiveVFX();
    FVFXDefinition* GetVFXDefinition(FName EffectName);
    bool ShouldCullVFX(const FVFXDefinition& VFXDef, FVector Location);
};