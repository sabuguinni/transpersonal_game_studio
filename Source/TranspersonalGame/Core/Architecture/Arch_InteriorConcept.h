#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Arch_InteriorConcept.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    FVector RelativeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    FRotator RelativeRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    EArch_InteriorElementType ElementType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    bool bIsEssential;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Element")
    float FunctionalRadius;

    FArch_InteriorElement()
    {
        ElementName = TEXT("Unknown Element");
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        ElementType = EArch_InteriorElementType::Decoration;
        bIsEssential = false;
        FunctionalRadius = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FString LayoutName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    EArch_ShelterType ShelterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FVector InteriorDimensions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    TArray<FArch_InteriorElement> InteriorElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FVector FirePitLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    TArray<FVector> SleepingAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    TArray<FVector> StorageLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FVector EntranceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    float AmbientLightLevel;

    FArch_InteriorLayout()
    {
        LayoutName = TEXT("Basic Cave Layout");
        ShelterType = EArch_ShelterType::CaveEntrance;
        InteriorDimensions = FVector(1000.0f, 800.0f, 300.0f);
        FirePitLocation = FVector(0.0f, 0.0f, -50.0f);
        EntranceLocation = FVector(500.0f, 0.0f, 0.0f);
        AmbientLightLevel = 0.1f;
    }
};

/**
 * Prehistoric Interior Architecture Concept Data Asset
 * Defines interior layouts, elements, and atmospheric details for Stone Age dwellings
 * Used as reference for shelter interiors and cave dwelling design
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArch_InteriorConcept : public UDataAsset
{
    GENERATED_BODY()

public:
    UArch_InteriorConcept();

    // Concept Art Reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concept Art")
    FString ConceptDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concept Art")
    UTexture2D* ConceptArtTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Concept Art")
    TArray<FString> DesignNotes;

    // Interior Layouts
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    TArray<FArch_InteriorLayout> InteriorLayouts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    TMap<EArch_ShelterType, FArch_InteriorLayout> DefaultLayouts;

    // Material References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> WallMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> FloorMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> ElementMaterials;

    // Lighting Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FirelightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FirelightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientIntensity;

    // Atmospheric Details
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float HumidityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float TemperatureRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<FString> AtmosphericElements;

    // Functionality
    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    FArch_InteriorLayout GetLayoutForShelterType(EArch_ShelterType ShelterType);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    TArray<FArch_InteriorElement> GetEssentialElements(EArch_ShelterType ShelterType);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    FVector GetOptimalFirePitLocation(const FArch_InteriorLayout& Layout);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    TArray<FVector> GenerateSleepingPositions(const FArch_InteriorLayout& Layout, int32 MaxOccupants);

    UFUNCTION(BlueprintCallable, Category = "Materials")
    UMaterialInterface* GetRandomWallMaterial();

    UFUNCTION(BlueprintCallable, Category = "Materials")
    UMaterialInterface* GetRandomFloorMaterial();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyInteriorLighting(const FArch_InteriorLayout& Layout, AActor* ShelterActor);

protected:
    void InitializeDefaultLayouts();
    void SetupPrehistoricMaterials();
    void ConfigureAtmosphericSettings();
};