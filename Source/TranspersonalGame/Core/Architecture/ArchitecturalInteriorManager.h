#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "ArchitecturalInteriorManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString LayoutName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> FurniturePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FString> FurnitureTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float RoomWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float RoomLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float RoomHeight;

    FArch_InteriorLayout()
    {
        LayoutName = TEXT("DefaultRoom");
        RoomWidth = 500.0f;
        RoomLength = 500.0f;
        RoomHeight = 300.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorProp
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    UStaticMesh* PropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    FVector RelativePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    FRotator RelativeRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    bool bIsInteractable;

    FArch_InteriorProp()
    {
        PropName = TEXT("GenericProp");
        PropMesh = nullptr;
        RelativePosition = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        bIsInteractable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalInteriorManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalInteriorManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    FArch_InteriorLayout CurrentLayout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    TArray<FArch_InteriorProp> InteriorProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    EBiomeType AssociatedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WallMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* FloorMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* CeilingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float TemperatureModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float HumidityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bHasShelterBonus;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void GenerateInteriorLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void SpawnInteriorProps();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void ClearInteriorProps();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void SetBiomeSpecificMaterials(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void AddInteriorProp(const FString& PropName, UStaticMesh* Mesh, FVector Position, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    bool IsPositionInsideInterior(FVector WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    float GetShelterProtectionValue() const;

    UFUNCTION(BlueprintCallable, Category = "Interior Management", CallInEditor = true)
    void PreviewInteriorLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void ApplyWeatheringEffects(float WeatheringIntensity);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    TArray<AActor*> GetInteriorActors() const;

private:
    UPROPERTY()
    TArray<UStaticMeshComponent*> SpawnedPropComponents;

    UPROPERTY()
    TArray<AActor*> SpawnedPropActors;

    void CreateDefaultLayout();
    void LoadBiomeSpecificProps(EBiomeType BiomeType);
    void SetupInteriorLighting();
    void ApplyAtmosphericEffects();
};