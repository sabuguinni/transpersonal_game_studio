#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Arch_InteriorManager.generated.h"

UENUM(BlueprintType)
enum class EArch_InteriorType : uint8
{
    CaveDwelling        UMETA(DisplayName = "Cave Dwelling"),
    TreeShelter         UMETA(DisplayName = "Tree Platform Shelter"),
    BoneShelter         UMETA(DisplayName = "Bone Frame Shelter"),
    CliffDwelling       UMETA(DisplayName = "Cliff Dwelling"),
    GroundHut           UMETA(DisplayName = "Ground Hut"),
    RockShelter         UMETA(DisplayName = "Rock Overhang Shelter")
};

UENUM(BlueprintType)
enum class EArch_InteriorElement : uint8
{
    FirePit             UMETA(DisplayName = "Fire Pit"),
    SleepingArea        UMETA(DisplayName = "Sleeping Area"),
    ToolStorage         UMETA(DisplayName = "Tool Storage"),
    FoodStorage         UMETA(DisplayName = "Food Storage"),
    CookingArea         UMETA(DisplayName = "Cooking Area"),
    WorkBench           UMETA(DisplayName = "Work Bench"),
    WaterContainer      UMETA(DisplayName = "Water Container"),
    HideRack            UMETA(DisplayName = "Hide Drying Rack")
};

USTRUCT(BlueprintType)
struct FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorType InteriorType = EArch_InteriorType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<EArch_InteriorElement> Elements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector InteriorSize = FVector(400.0f, 400.0f, 250.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float ComfortLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float WarmthLevel = 0.3f;

    FArch_InteriorLayout()
    {
        Elements.Add(EArch_InteriorElement::FirePit);
        Elements.Add(EArch_InteriorElement::SleepingArea);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_InteriorManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WallMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RoofMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* AmbientLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    FArch_InteriorLayout InteriorLayout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    TArray<UStaticMesh*> ElementMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    TArray<UMaterialInterface*> InteriorMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    float FireLightIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    FLinearColor FireLightColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    bool bHasActiveFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    float ShelterCondition = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    float WeatherProtection = 0.8f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void SetInteriorType(EArch_InteriorType NewType);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void AddInteriorElement(EArch_InteriorElement Element, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void RemoveInteriorElement(EArch_InteriorElement Element);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void UpdateFirePitState(bool bIsActive);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    float GetComfortLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    float GetWarmthLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    bool CanAccommodateOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void RepairShelter(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void DamageShelter(float DamageAmount);

    UFUNCTION(BlueprintPure, Category = "Interior Management")
    TArray<EArch_InteriorElement> GetAvailableElements() const;

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void GenerateInteriorLayout();

protected:
    UFUNCTION()
    void SpawnInteriorElements();

    UFUNCTION()
    void UpdateLighting();

    UFUNCTION()
    void ApplyInteriorMaterials();

    UFUNCTION()
    FVector GetElementSpawnLocation(EArch_InteriorElement Element) const;

    UFUNCTION()
    UStaticMesh* GetElementMesh(EArch_InteriorElement Element) const;

private:
    TArray<UStaticMeshComponent*> SpawnedElements;
    TArray<UPointLightComponent*> InteriorLights;
    
    void CleanupElements();
    void CalculateComfortMetrics();
    void UpdateShelterCondition(float DeltaTime);
};