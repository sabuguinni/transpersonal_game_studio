#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Quest_ResourceGatheringSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood"),
    Plant       UMETA(DisplayName = "Plant Fiber"),
    Water       UMETA(DisplayName = "Water"),
    Bone        UMETA(DisplayName = "Bone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResourceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RespawnTime;

    FQuest_ResourceData()
    {
        ResourceType = EQuest_ResourceType::Stone;
        Quantity = 1;
        ResourceName = TEXT("Stone");
        RespawnTime = 30.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourceNode : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourceNode();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ResourceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Resource")
    FQuest_ResourceData ResourceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Resource")
    bool bIsHarvestable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Resource")
    float HarvestTime;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Resource")
    bool bIsBeingHarvested;

    FTimerHandle RespawnTimerHandle;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    bool CanHarvest() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void StartHarvest(AActor* Harvester);

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void CompleteHarvest();

    UFUNCTION(BlueprintCallable, Category = "Quest Resource")
    void SetResourceType(EQuest_ResourceType NewType, int32 NewQuantity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Resource")
    void OnResourceHarvested(AActor* Harvester, const FQuest_ResourceData& Resource);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Resource")
    void OnResourceRespawned();

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    void RespawnResource();
    void SetupResourceMesh();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_ResourceGatheringSystem : public UObject
{
    GENERATED_BODY()

public:
    UQuest_ResourceGatheringSystem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_ResourceData> RequiredResources;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<EQuest_ResourceType, int32> CollectedResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    FString QuestDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    bool bQuestCompleted;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeGatheringQuest(const TArray<FQuest_ResourceData>& Resources);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AddResource(EQuest_ResourceType ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    float GetQuestProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FString GetProgressText() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest System")
    void OnQuestCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest System")
    void OnResourceAdded(EQuest_ResourceType ResourceType, int32 NewQuantity, int32 RequiredQuantity);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SpawnResourceNodes(UWorld* World, const TArray<FVector>& Locations);

protected:
    void CheckQuestCompletion();
};