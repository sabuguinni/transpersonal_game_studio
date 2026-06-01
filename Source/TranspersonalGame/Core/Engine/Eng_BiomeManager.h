#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 30000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActors = 4000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxDinosaurs = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurTypes;

    FEng_BiomeConfig()
    {
        AllowedDinosaurTypes = {"TRex", "Velociraptor", "Triceratops"};
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CurrentActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CurrentDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float ActorDensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    bool bIsOverloaded = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FEng_BiomeConfig> BiomeConfigs;

    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    TMap<EBiomeType, FEng_BiomeStats> BiomeStats;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool CanSpawnActorInBiome(EBiomeType BiomeType, bool bIsDinosaur = false) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeStats();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CleanupOverloadedBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FString> GetAllowedDinosaursForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void InitializeDefaultBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogBiomeStatus();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateInterval = 5.0f;

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    void InitializeBiomeConfigs();
    void ValidateBiomeLimits();
    FEng_BiomeConfig* GetBiomeConfig(EBiomeType BiomeType);
    void RemoveOldestActorsFromBiome(EBiomeType BiomeType, int32 NumToRemove);
};