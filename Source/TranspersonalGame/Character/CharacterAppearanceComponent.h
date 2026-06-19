#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterAppearanceComponent.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    VeryDark    UMETA(DisplayName = "VeryDark")
};

UENUM(BlueprintType)
enum class EChar_HairStyle : uint8
{
    Short       UMETA(DisplayName = "Short"),
    Long        UMETA(DisplayName = "Long"),
    Braided     UMETA(DisplayName = "Braided"),
    Shaved      UMETA(DisplayName = "Shaved"),
    Wild        UMETA(DisplayName = "Wild")
};

UENUM(BlueprintType)
enum class EChar_ClothingTier : uint8
{
    Naked       UMETA(DisplayName = "Naked - Starting state"),
    LeafWrap    UMETA(DisplayName = "Leaf Wrap - Basic coverage"),
    HideBasic   UMETA(DisplayName = "Hide Basic - Animal skin"),
    HideCrafted UMETA(DisplayName = "Hide Crafted - Stitched hides"),
    BoneArmor   UMETA(DisplayName = "Bone Armor - Bone reinforced")
};

USTRUCT(BlueprintType)
struct FChar_WoundMark
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Size = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Age = 0.0f; // 0=fresh, 1=fully healed scar
};

USTRUCT(BlueprintType)
struct FChar_AppearanceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_HairStyle HairStyle = EChar_HairStyle::Short;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.05f, 0.03f, 0.01f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingTier ClothingTier = EChar_ClothingTier::LeafWrap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float DirtLevel = 0.0f; // 0=clean, 1=caked in mud

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BloodLevel = 0.0f; // 0=none, 1=heavily bloodied

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FChar_WoundMark> WoundMarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bIsMale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyMassIndex = 0.5f; // 0=emaciated, 1=well-fed
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterAppearanceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterAppearanceComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_AppearanceProfile AppearanceProfile;

    /** Apply dirt accumulation over time (called each game hour) */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void AccumulateDirt(float DeltaHours);

    /** Apply blood splatter from combat damage */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplyBloodSplatter(float DamageAmount);

    /** Add a wound mark at a body location */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void AddWoundMark(FVector BodyLocation, float WoundSize);

    /** Age all wounds (heal over time) */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void AgeWounds(float DeltaHours);

    /** Wash off dirt and blood (e.g. swimming in river) */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void WashClean(float WashIntensity);

    /** Upgrade clothing tier when player crafts better gear */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void UpgradeClothing(EChar_ClothingTier NewTier);

    /** Get current visual dirtiness as a 0-1 float for material parameter */
    UFUNCTION(BlueprintPure, Category = "Appearance")
    float GetDirtinessMaterialParam() const;

    /** Get current blood level as a 0-1 float for material parameter */
    UFUNCTION(BlueprintPure, Category = "Appearance")
    float GetBloodMaterialParam() const;

    /** Body mass index driven by hunger — emaciation visible on mesh */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void UpdateBodyMassFromHunger(float HungerLevel);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float AccumulatedDirtTime = 0.0f;
};
