#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "World_CaveSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_CaveEntrance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave")
    bool bIsConnected;

    FWorld_CaveEntrance()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Depth = 500.0f;
        Width = 300.0f;
        BiomeType = EBiomeType::Savanna;
        bIsConnected = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_UndergroundTunnel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
    FVector EndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
    float TunnelWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
    float TunnelHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
    EBiomeType StartBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
    EBiomeType EndBiome;

    FWorld_UndergroundTunnel()
    {
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector::ZeroVector;
        TunnelWidth = 400.0f;
        TunnelHeight = 300.0f;
        StartBiome = EBiomeType::Savanna;
        EndBiome = EBiomeType::Forest;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CaveSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CaveSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveEntranceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* CaveCollision;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    TArray<FWorld_CaveEntrance> CaveEntrances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    TArray<FWorld_UndergroundTunnel> UndergroundTunnels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    int32 MaxCavesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    float CaveSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    float MinCaveDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    bool bAutoGenerateCaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    bool bConnectBiomes;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void GenerateCaveEntrances();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void CreateUndergroundTunnels();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void ConnectNearestCaves();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    FWorld_CaveEntrance CreateCaveAtLocation(FVector Location, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool IsCaveLocationValid(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    TArray<FWorld_CaveEntrance> GetCavesInBiome(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    float GetDistanceToCave(FVector Location);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Cave System")
    void RegenerateCaveSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Cave System")
    void ClearAllCaves();

private:
    void InitializeCaveSystem();
    void SetupCaveComponents();
    FVector GetRandomLocationInBiome(EBiomeType Biome);
    bool IsLocationUnderground(FVector Location);
};