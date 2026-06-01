#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_WorldArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 MaxActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 CurrentActors;

    FEng_WorldZone()
    {
        ZoneName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        Radius = 15000.0f;
        BiomeType = EBiomeType::Savanna;
        MaxActors = 1000;
        CurrentActors = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceLimits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPropsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    FEng_PerformanceLimits()
    {
        MaxTotalActors = 8000;
        MaxDinosaurs = 150;
        MaxPropsPerBiome = 1000;
        TargetFrameRate = 60.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_WorldArchitect : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "World Architect")
    void InitializeWorldZones();

    UFUNCTION(BlueprintCallable, Category = "World Architect")
    bool CanSpawnActorInZone(const FVector& Location, const FString& ActorType);

    UFUNCTION(BlueprintCallable, Category = "World Architect")
    FEng_WorldZone GetZoneAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "World Architect")
    void RegisterActorSpawned(const FVector& Location, const FString& ActorType);

    UFUNCTION(BlueprintCallable, Category = "World Architect")
    void CleanupExcessActors();

    UFUNCTION(BlueprintCallable, Category = "World Architect")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "World Architect")
    TArray<FEng_WorldZone> GetAllZones() const { return WorldZones; }

    UFUNCTION(BlueprintCallable, Category = "World Architect")
    FEng_PerformanceLimits GetPerformanceLimits() const { return PerformanceLimits; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Architect")
    TArray<FEng_WorldZone> WorldZones;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Architect")
    FEng_PerformanceLimits PerformanceLimits;

    UPROPERTY(BlueprintReadOnly, Category = "World Architect")
    bool bIsInitialized;

private:
    void CreateDefaultZones();
    void UpdateZoneActorCounts();
    FEng_WorldZone* FindZoneByLocation(const FVector& Location);
};