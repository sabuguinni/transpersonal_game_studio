#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "World_UndergroundSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_UndergroundType : uint8
{
    Cave            UMETA(DisplayName = "Cave"),
    Tunnel          UMETA(DisplayName = "Tunnel"),
    Cavern          UMETA(DisplayName = "Cavern"),
    UndergroundRiver UMETA(DisplayName = "Underground River"),
    Catacombs       UMETA(DisplayName = "Catacombs"),
    Mine            UMETA(DisplayName = "Mine"),
    Bunker          UMETA(DisplayName = "Bunker")
};

UENUM(BlueprintType)
enum class EWorld_UndergroundDepth : uint8
{
    Shallow         UMETA(DisplayName = "Shallow (0-10m)"),
    Medium          UMETA(DisplayName = "Medium (10-50m)"),
    Deep            UMETA(DisplayName = "Deep (50-200m)"),
    VeryDeep        UMETA(DisplayName = "Very Deep (200m+)")
};

USTRUCT(BlueprintType)
struct FWorld_UndergroundConnection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    class AWorld_UndergroundSystem* ConnectedSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    FVector ConnectionPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    float TunnelWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    bool bIsAccessible;

    FWorld_UndergroundConnection()
    {
        ConnectedSystem = nullptr;
        ConnectionPoint = FVector::ZeroVector;
        TunnelWidth = 200.0f;
        bIsAccessible = true;
    }
};

USTRUCT(BlueprintType)
struct FWorld_UndergroundResource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float Quality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bIsExtracted;

    FWorld_UndergroundResource()
    {
        ResourceType = TEXT("Stone");
        Quantity = 100;
        Quality = 1.0f;
        Location = FVector::ZeroVector;
        bIsExtracted = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUndergroundSystemEntered, class AWorld_UndergroundSystem*, UndergroundSystem, class APawn*, Pawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUndergroundSystemExited, class AWorld_UndergroundSystem*, UndergroundSystem, class APawn*, Pawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUndergroundSystemDiscovered, class AWorld_UndergroundSystem*, UndergroundSystem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResourceDiscovered, class AWorld_UndergroundSystem*, UndergroundSystem, const FWorld_UndergroundResource&, Resource, class APawn*, Discoverer);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_UndergroundSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_UndergroundSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* UndergroundMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* EntranceTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* SupportStructures;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* ResourceNodes;

    // Underground Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground System")
    EWorld_UndergroundType UndergroundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground System")
    EWorld_UndergroundDepth DepthLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground System")
    float SystemLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground System")
    float SystemWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground System")
    float SystemHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underground System")
    float DepthBelowSurface;

    // Environmental Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float AirQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float LightLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bHasWater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bHasVentilation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bIsStable;

    // System State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsDiscovered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsExplored;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsAccessible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsCollapsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsFlooded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float ExplorationProgress;

    // Connections and Resources
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
    TArray<FWorld_UndergroundConnection> Connections;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TArray<FWorld_UndergroundResource> Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 MaxResourceNodes;

    // Support Structures
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
    int32 SupportBeamCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
    bool bHasLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
    bool bHasRailTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structures")
    bool bHasElevator;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnUndergroundSystemEntered OnUndergroundSystemEntered;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnUndergroundSystemExited OnUndergroundSystemExited;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnUndergroundSystemDiscovered OnUndergroundSystemDiscovered;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnResourceDiscovered OnResourceDiscovered;

public:
    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Underground System")
    void InitializeUndergroundSystem();

    UFUNCTION(BlueprintCallable, Category = "Underground System")
    void GenerateUndergroundGeometry();

    UFUNCTION(BlueprintCallable, Category = "Underground System")
    void SetupEnvironmentalEffects();

    UFUNCTION(BlueprintCallable, Category = "Underground System")
    void SpawnSupportStructures();

    UFUNCTION(BlueprintCallable, Category = "Underground System")
    void SpawnResourceNodes();

    // Connection Management
    UFUNCTION(BlueprintCallable, Category = "Connections")
    void AddConnection(AWorld_UndergroundSystem* OtherSystem, FVector ConnectionPoint, float TunnelWidth);

    UFUNCTION(BlueprintCallable, Category = "Connections")
    void RemoveConnection(AWorld_UndergroundSystem* OtherSystem);

    UFUNCTION(BlueprintCallable, Category = "Connections")
    bool IsConnectedTo(AWorld_UndergroundSystem* OtherSystem) const;

    UFUNCTION(BlueprintCallable, Category = "Connections")
    TArray<AWorld_UndergroundSystem*> GetConnectedSystems() const;

    // Resource Management
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void AddResource(const FString& ResourceType, int32 Quantity, float Quality, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool ExtractResource(const FString& ResourceType, int32 Amount, APawn* Extractor);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    int32 GetResourceQuantity(const FString& ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FWorld_UndergroundResource> GetAvailableResources() const;

    // Environmental Functions
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalConditions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetWaterLevel(float NewWaterLevel);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void TriggerCollapse(float CollapseIntensity);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void TriggerFlood(float FloodLevel);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Underground System")
    FString GetSystemDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Underground System")
    float GetSystemDangerLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Underground System")
    bool CanPlayerEnter(APawn* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Underground System")
    void SetSystemAccessibility(bool bAccessible);

    UFUNCTION(BlueprintCallable, Category = "Underground System")
    float GetDepthInMeters() const;

protected:
    // Internal Functions
    void UpdateAmbientAudio(float DeltaTime);
    void UpdateStructuralIntegrity(float DeltaTime);
    void UpdateResourceGeneration(float DeltaTime);
    void UpdateWaterFlow(float DeltaTime);

    // Trigger Events
    UFUNCTION()
    void OnEntranceBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEntranceEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    // Internal state tracking
    float LastEnvironmentUpdate;
    float LastStructuralCheck;
    float LastResourceRegen;
    
    // Cached calculations
    float CachedDangerLevel;
    bool bDangerLevelDirty;
};

#include "World_UndergroundSystem.generated.h"