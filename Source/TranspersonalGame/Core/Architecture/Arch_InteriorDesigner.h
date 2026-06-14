#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Arch_InteriorDesigner.generated.h"

UENUM(BlueprintType)
enum class EArch_InteriorStyle : uint8
{
    CaveDwelling     UMETA(DisplayName = "Cave Dwelling"),
    RockShelter      UMETA(DisplayName = "Rock Shelter"),
    TribalHut        UMETA(DisplayName = "Tribal Hut"),
    SacredSpace      UMETA(DisplayName = "Sacred Space")
};

UENUM(BlueprintType)
enum class EArch_PropType : uint8
{
    FirePit          UMETA(DisplayName = "Fire Pit"),
    SleepingArea     UMETA(DisplayName = "Sleeping Area"),
    ToolStorage      UMETA(DisplayName = "Tool Storage"),
    FoodStorage      UMETA(DisplayName = "Food Storage"),
    WaterContainer   UMETA(DisplayName = "Water Container"),
    WorkBench        UMETA(DisplayName = "Work Bench"),
    RitualAltar      UMETA(DisplayName = "Ritual Altar"),
    WallArt          UMETA(DisplayName = "Wall Art")
};

USTRUCT(BlueprintType)
struct FArch_InteriorProp
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_PropType PropType = EArch_PropType::FirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float UsageRadius = 150.0f;

    FArch_InteriorProp()
    {
        PropType = EArch_PropType::FirePit;
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        bIsActive = true;
        UsageRadius = 150.0f;
    }
};

USTRUCT(BlueprintType)
struct FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorStyle Style = EArch_InteriorStyle::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FArch_InteriorProp> Props;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector InteriorBounds = FVector(800.0f, 800.0f, 400.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasNaturalLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasArtificialLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float AmbientTemperature = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float Humidity = 0.6f;

    FArch_InteriorLayout()
    {
        Style = EArch_InteriorStyle::CaveDwelling;
        InteriorBounds = FVector(800.0f, 800.0f, 400.0f);
        bHasNaturalLighting = true;
        bHasArtificialLighting = true;
        AmbientTemperature = 18.0f;
        Humidity = 0.6f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_InteriorDesigner : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorDesigner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interior")
    USceneComponent* InteriorRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interior")
    TArray<UStaticMeshComponent*> PropMeshes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interior")
    TArray<UPointLightComponent*> LightSources;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interior")
    UAudioComponent* AmbientAudio;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FArch_InteriorLayout Layout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bAutoGenerateLayout = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    int32 MaxProps = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float PropDensity = 0.3f;

    // Lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FireLightColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FireLightIntensity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FireLightRadius = 500.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Interior")
    void GenerateInteriorLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void ClearInterior();

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void AddProp(EArch_PropType PropType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void RemoveProp(int32 PropIndex);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetInteriorStyle(EArch_InteriorStyle NewStyle);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    bool IsLocationSuitableForProp(const FVector& Location, EArch_PropType PropType) const;

    UFUNCTION(BlueprintCallable, Category = "Interior")
    TArray<FVector> GetOptimalPropLocations(EArch_PropType PropType, int32 MaxLocations = 5) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAmbientAudio();

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnInteriorGenerated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnPropAdded(EArch_PropType PropType, const FVector& Location);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior")
    void OnPropRemoved(int32 PropIndex);

private:
    void GenerateCaveDwellingLayout();
    void GenerateRockShelterLayout();
    void GenerateTribalHutLayout();
    void GenerateSacredSpaceLayout();

    void CreatePropMesh(const FArch_InteriorProp& PropData);
    void CreateFirePit(const FVector& Location);
    void CreateSleepingArea(const FVector& Location, const FRotator& Rotation);
    void CreateToolStorage(const FVector& Location);
    void CreateWorkBench(const FVector& Location, const FRotator& Rotation);

    void SetupFireLighting(const FVector& FireLocation);
    void SetupAmbientLighting();

    // Interior state
    UPROPERTY()
    TArray<FArch_InteriorProp> ActiveProps;

    UPROPERTY()
    bool bIsLayoutGenerated = false;

    UPROPERTY()
    float LastGenerationTime = 0.0f;
};