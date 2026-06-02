#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Arch_UndergroundNetworkSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ChamberType : uint8
{
    SmallChamber,
    LargeChamber,
    CrystalCavern,
    WaterChamber,
    LavaTube,
    AncientRuin,
    BoneYard,
    MushroomGrove
};

UENUM(BlueprintType)
enum class EArch_TunnelType : uint8
{
    NarrowPassage,
    WideTunnel,
    CrawlSpace,
    WaterTunnel,
    LavaFlow,
    CollapseZone
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ChamberData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArch_ChamberType ChamberType = EArch_ChamberType::SmallChamber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Height = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Humidity = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasCrystals = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsStable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> ConnectedTunnels;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_TunnelData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArch_TunnelType TunnelType = EArch_TunnelType::NarrowPassage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EndLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Width = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Height = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Length = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsBlocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StabilityFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FromChamber = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ToChamber = -1;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_UndergroundNetworkSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_UndergroundNetworkSystem();

protected:
    virtual void BeginPlay() override;

    // Network Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground Network")
    TArray<FArch_ChamberData> Chambers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground Network")
    TArray<FArch_TunnelData> Tunnels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground Network")
    float MaxNetworkDepth = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground Network")
    float MinChamberDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground Network")
    int32 MaxChambers = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground Network")
    int32 MaxTunnels = 20;

    // Environmental Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BaseTemperature = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float DepthTemperatureIncrease = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BaseHumidity = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bGenerateAmbientSounds = true;

    // Visual Components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<UStaticMeshComponent*> ChamberMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<UStaticMeshComponent*> TunnelMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<UPointLightComponent*> AmbientLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<UParticleSystemComponent*> EnvironmentalEffects;

public:
    // Network Generation
    UFUNCTION(BlueprintCallable, Category = "Underground Network")
    void GenerateUndergroundNetwork(const FVector& CenterLocation, int32 NumChambers = 8);

    UFUNCTION(BlueprintCallable, Category = "Underground Network")
    void CreateChamber(const FVector& Location, EArch_ChamberType ChamberType, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Underground Network")
    void ConnectChambers(int32 ChamberA, int32 ChamberB, EArch_TunnelType TunnelType = EArch_TunnelType::NarrowPassage);

    UFUNCTION(BlueprintCallable, Category = "Underground Network")
    void PopulateChamber(int32 ChamberIndex);

    // Environmental Control
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalConditions();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float CalculateTemperatureAtDepth(float Depth) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float CalculateHumidityInChamber(int32 ChamberIndex) const;

    // Navigation and Access
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    TArray<int32> FindPathBetweenChambers(int32 StartChamber, int32 EndChamber) const;

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    int32 FindNearestChamber(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    bool IsChamberAccessible(int32 ChamberIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    TArray<int32> GetConnectedChambers(int32 ChamberIndex) const;

    // Visual Management
    UFUNCTION(BlueprintCallable, Category = "Visual")
    void CreateChamberVisuals(int32 ChamberIndex);

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void CreateTunnelVisuals(int32 TunnelIndex);

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void UpdateLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void AddEnvironmentalEffects(int32 ChamberIndex);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ClearNetwork();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ValidateNetworkIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString GetNetworkStatistics() const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Underground Network")
    const TArray<FArch_ChamberData>& GetChambers() const { return Chambers; }

    UFUNCTION(BlueprintPure, Category = "Underground Network")
    const TArray<FArch_TunnelData>& GetTunnels() const { return Tunnels; }

    UFUNCTION(BlueprintPure, Category = "Underground Network")
    int32 GetChamberCount() const { return Chambers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Underground Network")
    int32 GetTunnelCount() const { return Tunnels.Num(); }

private:
    // Internal Generation Helpers
    void GenerateChamberLayout(const FVector& CenterLocation, int32 NumChambers);
    void GenerateTunnelConnections();
    void AssignChamberTypes();
    void CalculateEnvironmentalFactors();
    
    // Validation Helpers
    bool IsLocationValid(const FVector& Location) const;
    bool CanConnectChambers(int32 ChamberA, int32 ChamberB) const;
    float CalculateDistance(const FVector& A, const FVector& B) const;
    
    // Visual Helpers
    void CreateChamberGeometry(const FArch_ChamberData& Chamber);
    void CreateTunnelGeometry(const FArch_TunnelData& Tunnel);
    void SetupChamberLighting(int32 ChamberIndex);
    void SetupEnvironmentalParticles(int32 ChamberIndex);
};