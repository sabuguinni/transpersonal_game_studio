#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Arch_ShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    Cave           UMETA(DisplayName = "Cave Shelter"),
    Platform       UMETA(DisplayName = "Elevated Platform"),
    LeanTo         UMETA(DisplayName = "Lean-To Structure"),
    StoneCircle    UMETA(DisplayName = "Stone Circle"),
    Underground    UMETA(DisplayName = "Underground Burrow")
};

USTRUCT(BlueprintType)
struct FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionValue = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureBonus = 5.0f;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::Cave;
        ProtectionValue = 0.5f;
        MaxOccupants = 2;
        bHasFirePit = false;
        bHasStorage = false;
        TemperatureBonus = 5.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SupportStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterData ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    EBiomeType AssignedBiome = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    float ShelterRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TSoftObjectPtr<UStaticMesh> CaveMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TSoftObjectPtr<UStaticMesh> PlatformMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TSoftObjectPtr<UStaticMesh> StoneMesh;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void InitializeShelter(EArch_ShelterType Type, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterMesh(UStaticMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetProtectionValue() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanAccommodateOccupants(int32 RequestedOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void AddFirePit();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void AddStorageArea();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    FVector GetShelterCenter() const;

    UFUNCTION(BlueprintPure, Category = "Shelter")
    EArch_ShelterType GetShelterType() const { return ShelterData.ShelterType; }

    UFUNCTION(BlueprintPure, Category = "Shelter")
    bool HasFirePit() const { return ShelterData.bHasFirePit; }

    UFUNCTION(BlueprintPure, Category = "Shelter")
    bool HasStorage() const { return ShelterData.bHasStorage; }

private:
    void UpdateMeshBasedOnType();
    void ApplyBiomeSpecificMaterials();
    void SetupCollisionAndPhysics();
};