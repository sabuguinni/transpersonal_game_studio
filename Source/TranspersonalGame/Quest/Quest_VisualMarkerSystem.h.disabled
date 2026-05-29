#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Quest_VisualMarkerSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_MarkerType : uint8
{
    MainQuest       UMETA(DisplayName = "Main Quest"),
    SideQuest       UMETA(DisplayName = "Side Quest"),
    CraftingQuest   UMETA(DisplayName = "Crafting Quest"),
    ExplorationQuest UMETA(DisplayName = "Exploration Quest"),
    SurvivalQuest   UMETA(DisplayName = "Survival Quest"),
    HuntingQuest    UMETA(DisplayName = "Hunting Quest"),
    CompletionMarker UMETA(DisplayName = "Completion Marker")
};

UENUM(BlueprintType)
enum class EQuest_MarkerState : uint8
{
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Locked          UMETA(DisplayName = "Locked")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MarkerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    EQuest_MarkerType MarkerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    EQuest_MarkerState MarkerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FLinearColor MarkerColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    bool bIsVisible;

    FQuest_MarkerData()
    {
        QuestID = TEXT("");
        QuestTitle = TEXT("");
        QuestDescription = TEXT("");
        MarkerType = EQuest_MarkerType::SideQuest;
        MarkerState = EQuest_MarkerState::Available;
        WorldLocation = FVector::ZeroVector;
        MarkerColor = FLinearColor::White;
        InteractionRadius = 500.0f;
        bIsVisible = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_VisualMarker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_VisualMarker();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* MarkerLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FQuest_MarkerData MarkerData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    float PulseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    float PulseIntensityMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    float PulseIntensityMax;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void UpdateMarkerState(EQuest_MarkerState NewState);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void SetMarkerVisibility(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void SetMarkerColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    bool IsPlayerInRange(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    FQuest_MarkerData GetMarkerData() const { return MarkerData; }

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void SetMarkerData(const FQuest_MarkerData& NewData);

private:
    float CurrentPulseTime;
    void UpdatePulseEffect(float DeltaTime);
    void ApplyStateVisuals();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_VisualMarkerSystem : public UObject
{
    GENERATED_BODY()

public:
    UQuest_VisualMarkerSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    static UQuest_VisualMarkerSystem* GetInstance(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    AQuest_VisualMarker* CreateQuestMarker(const FQuest_MarkerData& MarkerData, UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    void RemoveQuestMarker(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    AQuest_VisualMarker* FindQuestMarker(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    TArray<AQuest_VisualMarker*> GetAllQuestMarkers();

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    TArray<AQuest_VisualMarker*> GetQuestMarkersByType(EQuest_MarkerType MarkerType);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    TArray<AQuest_VisualMarker*> GetQuestMarkersInRange(FVector Location, float Range);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    void UpdateAllMarkersVisibility(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    void RefreshMarkerStates();

protected:
    UPROPERTY()
    TArray<AQuest_VisualMarker*> ActiveMarkers;

    UPROPERTY()
    TMap<FString, AQuest_VisualMarker*> MarkerRegistry;

    static UQuest_VisualMarkerSystem* Instance;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    void CreateBiomeQuestMarkers(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    void CreateTutorialMarkers(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker System")
    void CreateSurvivalQuestMarkers(UWorld* World);
};