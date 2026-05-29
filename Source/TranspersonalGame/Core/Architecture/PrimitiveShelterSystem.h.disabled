#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "PrimitiveShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    Cave UMETA(DisplayName = "Cave Dwelling"),
    Wooden UMETA(DisplayName = "Wooden Structure"),
    Stone UMETA(DisplayName = "Stone Construction"),
    Elevated UMETA(DisplayName = "Elevated Platform"),
    Ceremonial UMETA(DisplayName = "Ceremonial Site")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::Wooden;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FString ShelterName = TEXT("Unnamed Shelter");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 Capacity = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::Wooden;
        ShelterName = TEXT("Basic Shelter");
        Capacity = 4;
        StructuralIntegrity = 100.0f;
        bHasFirePit = false;
        bHasStorage = false;
        WeatherProtection = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrimitiveShelter : public AActor
{
    GENERATED_BODY()

public:
    APrimitiveShelter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Data")
    FArch_ShelterData ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WoodenMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* CaveMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void InitializeShelter(EArch_ShelterType Type, const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterMesh(EArch_ShelterType Type);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanAccommodateOccupants(int32 NumOccupants) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void DamageShelter(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RepairShelter(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtectionLevel() const;

    UFUNCTION(BlueprintPure, Category = "Shelter")
    FArch_ShelterData GetShelterData() const { return ShelterData; }

    virtual void Tick(float DeltaTime) override;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPrimitiveShelterManager : public UObject
{
    GENERATED_BODY()

public:
    UPrimitiveShelterManager();

    UFUNCTION(BlueprintCallable, Category = "Shelter Manager")
    static APrimitiveShelter* CreateShelterAtLocation(UWorld* World, EArch_ShelterType Type, 
        FVector Location, FRotator Rotation, const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Shelter Manager")
    static TArray<APrimitiveShelter*> GetAllSheltersInRadius(UWorld* World, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Shelter Manager")
    static APrimitiveShelter* FindNearestShelter(UWorld* World, FVector Location, float MaxDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Shelter Manager")
    static bool ValidateShelterPlacement(UWorld* World, FVector Location, float MinDistance = 1000.0f);

protected:
    static TArray<FVector> GetDefaultShelterLocations();
    static EArch_ShelterType GetRecommendedShelterType(FVector Location);
};