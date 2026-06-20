#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Combat          UMETA(DisplayName = "Combat"),
    Weather         UMETA(DisplayName = "Weather"),
    Crafting        UMETA(DisplayName = "Crafting")
};

USTRUCT(BlueprintType)
struct FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName EffectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD0_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD1_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD2_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bCastShadows;

    FVFX_EffectConfig()
        : EffectID(NAME_None)
        , Category(EVFX_EffectCategory::Environment)
        , LOD0_Distance(500.f)
        , LOD1_Distance(1500.f)
        , LOD2_Distance(4000.f)
        , bCastShadows(false)
    {}
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_Manager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_Manager();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterEffect(const FVFX_EffectConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(FName EffectID, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(FName EffectID);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateLOD(FVector PlayerLocation);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    TArray<FVFX_EffectConfig> RegisteredEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float GlobalLODScalar;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    TMap<FName, FVFX_EffectConfig> EffectRegistry;
};
