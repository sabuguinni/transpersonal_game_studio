#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_ShelterBuilder.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    LeanTo UMETA(DisplayName = "Lean-To"),
    AFrame UMETA(DisplayName = "A-Frame Hut"),
    RoundHut UMETA(DisplayName = "Round Hut"),
    RockShelter UMETA(DisplayName = "Rock Shelter"),
    PitHouse UMETA(DisplayName = "Pit House")
};

UENUM(BlueprintType)
enum class EArch_BuildingMaterial : uint8
{
    Wood UMETA(DisplayName = "Wood"),
    Stone UMETA(DisplayName = "Stone"),
    Thatch UMETA(DisplayName = "Thatch"),
    Hide UMETA(DisplayName = "Animal Hide"),
    Mud UMETA(DisplayName = "Mud Brick")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::LeanTo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_BuildingMaterial PrimaryMaterial = EArch_BuildingMaterial::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float Width = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float Length = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float Height = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 SupportPosts = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorageArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float StructuralIntegrity = 1.0f;

    FArch_ShelterConfiguration()
    {
        ShelterType = EArch_ShelterType::LeanTo;
        PrimaryMaterial = EArch_BuildingMaterial::Wood;
        Width = 300.0f;
        Length = 400.0f;
        Height = 250.0f;
        SupportPosts = 4;
        bHasFirePit = true;
        bHasStorageArea = false;
        StructuralIntegrity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterBuilder : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterBuilder();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FrameMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RoofMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WallMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FArch_ShelterConfiguration ShelterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WoodMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ThatchMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* HideMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MudMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void BuildShelter();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetPrimaryMaterial(EArch_BuildingMaterial NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void UpdateStructuralIntegrity(float IntegrityChange);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanBuildAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    TArray<FVector> GetRequiredResourceLocations() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetBuildProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void AddFirePit();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void AddStorageArea();

private:
    void BuildLeanTo();
    void BuildAFrame();
    void BuildRoundHut();
    void BuildRockShelter();
    void BuildPitHouse();
    
    void ApplyMaterialToComponents();
    void SetupShelterCollision();
    void CreateSupportStructure();
    
    float BuildProgress = 0.0f;
    bool bIsBuilding = false;
};