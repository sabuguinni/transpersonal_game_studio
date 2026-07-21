#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "SharedTypes.h"
#include "EnvArt_WeatheredBoulder.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_BoulderType : uint8
{
    Granite UMETA(DisplayName = "Granite"),
    Sandstone UMETA(DisplayName = "Sandstone"),
    Limestone UMETA(DisplayName = "Limestone"),
    Volcanic UMETA(DisplayName = "Volcanic Rock"),
    Shale UMETA(DisplayName = "Shale")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatheringLevel : uint8
{
    Fresh UMETA(DisplayName = "Fresh Rock"),
    LightWeathering UMETA(DisplayName = "Light Weathering"),
    ModerateWeathering UMETA(DisplayName = "Moderate Weathering"),
    HeavyWeathering UMETA(DisplayName = "Heavy Weathering"),
    Ancient UMETA(DisplayName = "Ancient Weathered")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_WeatheredBoulder : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AEnvArt_WeatheredBoulder();

protected:
    virtual void BeginPlay() override;

    // Boulder Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    EEnvArt_BoulderType BoulderType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    EEnvArt_WeatheringLevel WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float SizeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    bool bHasMossGrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    bool bHasLichenGrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    bool bCanBeClimbed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    bool bProvidesShade;

    // Environmental Storytelling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FString StoryContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    bool bHasToolMarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    bool bHasAnimalScratchMarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    bool bNearWaterSource;

    // Interaction Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* ClimbingTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* ShadeTrigger;

    // Material Variants
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<class UMaterialInterface*> BoulderMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<class UMaterialInterface*> MossMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<class UMaterialInterface*> LichenMaterials;

public:
    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Boulder Functions")
    void ApplyWeatheringEffects();

    UFUNCTION(BlueprintCallable, Category = "Boulder Functions")
    void SetBoulderType(EEnvArt_BoulderType NewType);

    UFUNCTION(BlueprintCallable, Category = "Boulder Functions")
    void SetWeatheringLevel(EEnvArt_WeatheringLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Boulder Functions")
    void ToggleMossGrowth(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Boulder Functions")
    void ToggleLichenGrowth(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Boulder Functions")
    FVector GetClimbingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Boulder Functions")
    bool CanProvideShade() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Boulder Events")
    void OnPlayerClimbed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Boulder Events")
    void OnPlayerEnteredShade();

    UFUNCTION(BlueprintImplementableEvent, Category = "Boulder Events")
    void OnPlayerExitedShade();

protected:
    UFUNCTION()
    void OnClimbingTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnShadeTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnShadeTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void InitializeBoulderMesh();
    void UpdateMaterialBasedOnProperties();
    void SetupCollisionAndTriggers();
};