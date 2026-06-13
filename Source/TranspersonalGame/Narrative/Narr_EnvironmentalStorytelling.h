#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_EnvironmentalStorytelling.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_EnvironmentalStory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FString StoryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    TArray<FString> VisualClues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    bool bIsDiscovered;

    FNarr_EnvironmentalStory()
    {
        StoryID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        bIsDiscovered = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_AncientSite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ancient Site")
    FString SiteID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ancient Site")
    FString SiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ancient Site")
    TArray<FString> CavePaintings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ancient Site")
    TArray<FString> BoneRemains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ancient Site")
    TArray<FString> ToolFragments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ancient Site")
    FVector SiteLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ancient Site")
    float ExplorationRadius;

    FNarr_AncientSite()
    {
        SiteID = TEXT("");
        SiteName = TEXT("");
        SiteLocation = FVector::ZeroVector;
        ExplorationRadius = 1000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_EnvironmentalStorytelling : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_EnvironmentalStorytelling();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Environmental Stories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Stories")
    TArray<FNarr_EnvironmentalStory> EnvironmentalStories;

    // Ancient Sites
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ancient Sites")
    TArray<FNarr_AncientSite> AncientSites;

    // Discovery System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
    float DiscoveryCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
    float PlayerDetectionRadius;

    // Story Progression
    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CheckForNearbyStories();

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void DiscoverStory(const FString& StoryID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void ExploreAncientSite(const FString& SiteID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    TArray<FString> GetDiscoveredStories() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    FNarr_EnvironmentalStory GetStoryByID(const FString& StoryID) const;

    // Cave Painting System
    UFUNCTION(BlueprintCallable, Category = "Cave Paintings")
    void RegisterCavePainting(const FString& SiteID, const FString& PaintingDescription);

    UFUNCTION(BlueprintCallable, Category = "Cave Paintings")
    TArray<FString> GetCavePaintingsAtSite(const FString& SiteID) const;

    // Bone Analysis
    UFUNCTION(BlueprintCallable, Category = "Bone Analysis")
    void AnalyzeBoneRemains(const FString& SiteID, const FString& BoneDescription);

    UFUNCTION(BlueprintCallable, Category = "Bone Analysis")
    TArray<FString> GetBoneRemainsAtSite(const FString& SiteID) const;

    // Tool Fragment Discovery
    UFUNCTION(BlueprintCallable, Category = "Tool Fragments")
    void DiscoverToolFragment(const FString& SiteID, const FString& ToolDescription);

    UFUNCTION(BlueprintCallable, Category = "Tool Fragments")
    TArray<FString> GetToolFragmentsAtSite(const FString& SiteID) const;

private:
    UPROPERTY()
    float LastDiscoveryCheck;

    UPROPERTY()
    TArray<FString> DiscoveredStoryIDs;

    void InitializeDefaultStories();
    void InitializeDefaultSites();
    bool IsPlayerNearLocation(const FVector& Location, float Radius) const;
    APawn* GetPlayerPawn() const;
};