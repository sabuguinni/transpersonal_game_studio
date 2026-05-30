#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Crowd_HerdManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FString HerdID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    int32 MemberCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FString SpeciesType;

    FCrowd_HerdData()
    {
        HerdID = TEXT("DefaultHerd");
        CenterLocation = FVector::ZeroVector;
        HerdRadius = 1000.0f;
        MemberCount = 0;
        SpeciesType = TEXT("Triceratops");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_HerdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_HerdManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Management")
    TArray<FCrowd_HerdData> ActiveHerds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Management")
    float HerdUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Management")
    int32 MaxHerdsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Management")
    float HerdSpawnRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* HerdBounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* HerdMarker;

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void CreateHerd(const FString& HerdID, const FVector& Location, const FString& Species, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void UpdateHerdPositions();

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void SpawnHerdMember(const FString& HerdID, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    FCrowd_HerdData GetHerdData(const FString& HerdID);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void RemoveHerd(const FString& HerdID);

private:
    float LastUpdateTime;
    
    void InitializeBiomeHerds();
    void UpdateHerdBehavior(FCrowd_HerdData& HerdData);
    FVector GetRandomLocationInHerd(const FCrowd_HerdData& HerdData);
};