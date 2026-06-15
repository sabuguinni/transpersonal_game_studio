#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "EnvArt_FallenLogAsset.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_LogDecayState : uint8
{
    Fresh       UMETA(DisplayName = "Fresh"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Mossy       UMETA(DisplayName = "Mossy"),
    Rotting     UMETA(DisplayName = "Rotting"),
    Hollow      UMETA(DisplayName = "Hollow")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LogProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    float LogLength = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    float LogDiameter = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    EEnvArt_LogDecayState DecayState = EEnvArt_LogDecayState::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    bool bHasMoss = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    bool bHasLichen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    float MossIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    bool bCanHidePlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    bool bCanClimbOver = true;
};

/**
 * Environment Artist fallen log asset for Cretaceous forest environments
 * Provides realistic weathered logs with moss/lichen growth for atmospheric storytelling
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_FallenLogAsset : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AEnvArt_FallenLogAsset();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* LogMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Configuration")
    FEnvArt_LogProperties LogProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* FreshLogMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* WeatheredLogMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* MossyLogMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* RottenLogMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* HollowLogMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetDecayState(EEnvArt_LogDecayState NewDecayState);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetMossIntensity(float NewMossIntensity);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void RandomizeLogProperties();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool CanPlayerHideBehind() const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FVector GetClimbOverPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyEnvironmentalWeathering(float DeltaTime);

private:
    void UpdateLogMaterial();
    void SetupCollisionForDecayState();
    
    UPROPERTY()
    float WeatheringTimer = 0.0f;
};