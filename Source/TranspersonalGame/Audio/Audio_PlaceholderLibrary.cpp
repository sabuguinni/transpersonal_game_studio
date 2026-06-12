#include "Audio_PlaceholderLibrary.h"

UAudio_PlaceholderLibrary::UAudio_PlaceholderLibrary()
{
    InitializePrehistoricAudioSpecs();
}

void UAudio_PlaceholderLibrary::InitializePrehistoricAudioSpecs()
{
    // TODO_TTS_FAILED: These specs define narration and audio that should be generated manually
    
    // Dinosaur Audio Specifications
    DinosaurAudioSpecs.Empty();
    
    FAudio_PlaceholderSpec TRexRoar;
    TRexRoar.AudioID = TEXT("TRex_Roar_Territorial");
    TRexRoar.Description = TEXT("Deep, thunderous roar of a T-Rex asserting dominance. Should cause ground trembling effect and fear response in smaller creatures. Low frequency rumble with sharp attack.");
    TRexRoar.SuggestedFreesoundQuery = TEXT("tyrannosaurus rex roar deep prehistoric");
    TRexRoar.TargetDuration = 4.5f;
    TRexRoar.RecommendedVolume = 1.8f;
    TRexRoar.AudioCategory = TEXT("Dinosaur");
    DinosaurAudioSpecs.Add(TRexRoar);
    
    FAudio_PlaceholderSpec RaptorPack;
    RaptorPack.AudioID = TEXT("Raptor_Pack_Communication");
    RaptorPack.Description = TEXT("Sharp, intelligent calls between hunting raptors. Quick chirps and clicks for pack coordination. Should convey intelligence and coordination threat.");
    RaptorPack.SuggestedFreesoundQuery = TEXT("raptor bird call hunting pack communication");
    RaptorPack.TargetDuration = 3.0f;
    RaptorPack.RecommendedVolume = 1.2f;
    RaptorPack.AudioCategory = TEXT("Dinosaur");
    DinosaurAudioSpecs.Add(RaptorPack);
    
    FAudio_PlaceholderSpec BrachiosaurusCall;
    BrachiosaurusCall.AudioID = TEXT("Brachiosaurus_Gentle_Call");
    BrachiosaurusCall.Description = TEXT("Low, whale-like call of a peaceful herbivore. Gentle and non-threatening, used for long-distance communication with herd members.");
    BrachiosaurusCall.SuggestedFreesoundQuery = TEXT("whale call low frequency gentle herbivore");
    BrachiosaurusCall.TargetDuration = 6.0f;
    BrachiosaurusCall.RecommendedVolume = 1.0f;
    BrachiosaurusCall.AudioCategory = TEXT("Dinosaur");
    DinosaurAudioSpecs.Add(BrachiosaurusCall);
    
    // Environment Audio Specifications
    EnvironmentAudioSpecs.Empty();
    
    FAudio_PlaceholderSpec PrehistoricForest;
    PrehistoricForest.AudioID = TEXT("Prehistoric_Forest_Ambient");
    PrehistoricForest.Description = TEXT("Dense prehistoric forest ambience with ancient insects, distant bird calls, wind through primitive vegetation, and occasional branch creaks. Should feel ancient and untamed.");
    PrehistoricForest.SuggestedFreesoundQuery = TEXT("forest ambience prehistoric ancient insects wind");
    PrehistoricForest.TargetDuration = 120.0f;
    PrehistoricForest.RecommendedVolume = 0.6f;
    PrehistoricForest.bShouldLoop = true;
    PrehistoricForest.AudioCategory = TEXT("Environment");
    EnvironmentAudioSpecs.Add(PrehistoricForest);
    
    FAudio_PlaceholderSpec PrimordialRiver;
    PrimordialRiver.AudioID = TEXT("Primordial_River_Flow");
    PrimordialRiver.Description = TEXT("Clean, fast-flowing river in prehistoric landscape. Water rushing over rocks with occasional splash from unseen creatures drinking or crossing.");
    PrimordialRiver.SuggestedFreesoundQuery = TEXT("river flow clean water rocks prehistoric");
    PrimordialRiver.TargetDuration = 90.0f;
    PrimordialRiver.RecommendedVolume = 0.8f;
    PrimordialRiver.bShouldLoop = true;
    PrimordialRiver.AudioCategory = TEXT("Environment");
    EnvironmentAudioSpecs.Add(PrimordialRiver);
    
    // Survival Audio Specifications
    SurvivalAudioSpecs.Empty();
    
    FAudio_PlaceholderSpec PrimitiveFire;
    PrimitiveFire.AudioID = TEXT("Primitive_Campfire_Crackle");
    PrimitiveFire.Description = TEXT("Crackling of primitive campfire with dry wood and tinder. Should convey warmth, safety, and human presence in dangerous wilderness.");
    PrimitiveFire.SuggestedFreesoundQuery = TEXT("campfire crackle wood burning primitive");
    PrimitiveFire.TargetDuration = 60.0f;
    PrimitiveFire.RecommendedVolume = 0.7f;
    PrimitiveFire.bShouldLoop = true;
    PrimitiveFire.AudioCategory = TEXT("Survival");
    SurvivalAudioSpecs.Add(PrimitiveFire);
    
    FAudio_PlaceholderSpec StoneCrafting;
    StoneCrafting.AudioID = TEXT("Stone_Tool_Crafting");
    StoneCrafting.Description = TEXT("Sharp impacts of stone on stone, creating primitive tools. Rhythmic knapping sounds with occasional scraping as edges are refined.");
    StoneCrafting.SuggestedFreesoundQuery = TEXT("stone knapping flint crafting primitive tools");
    StoneCrafting.TargetDuration = 8.0f;
    StoneCrafting.RecommendedVolume = 1.0f;
    StoneCrafting.AudioCategory = TEXT("Survival");
    SurvivalAudioSpecs.Add(StoneCrafting);
    
    // Combat Audio Specifications
    CombatAudioSpecs.Empty();
    
    FAudio_PlaceholderSpec SpearThrust;
    SpearThrust.AudioID = TEXT("Primitive_Spear_Thrust");
    SpearThrust.Description = TEXT("Whoosh of primitive spear cutting through air, followed by impact sound. Should convey the weight and primitive construction of stone-tipped weapon.");
    SpearThrust.SuggestedFreesoundQuery = TEXT("spear thrust whoosh impact primitive weapon");
    SpearThrust.TargetDuration = 1.5f;
    SpearThrust.RecommendedVolume = 1.1f;
    SpearThrust.AudioCategory = TEXT("Combat");
    CombatAudioSpecs.Add(SpearThrust);
    
    FAudio_PlaceholderSpec HeavyFootsteps;
    HeavyFootsteps.AudioID = TEXT("Heavy_Creature_Footsteps");
    HeavyFootsteps.Description = TEXT("Thunderous footsteps of massive dinosaur approaching. Ground impact with debris falling, should create sense of impending danger and scale.");
    HeavyFootsteps.SuggestedFreesoundQuery = TEXT("heavy footsteps giant creature ground impact");
    HeavyFootsteps.TargetDuration = 2.0f;
    HeavyFootsteps.RecommendedVolume = 1.5f;
    HeavyFootsteps.AudioCategory = TEXT("Combat");
    CombatAudioSpecs.Add(HeavyFootsteps);
}

TArray<FAudio_PlaceholderSpec> UAudio_PlaceholderLibrary::GetAllAudioSpecs() const
{
    TArray<FAudio_PlaceholderSpec> AllSpecs;
    AllSpecs.Append(DinosaurAudioSpecs);
    AllSpecs.Append(EnvironmentAudioSpecs);
    AllSpecs.Append(SurvivalAudioSpecs);
    AllSpecs.Append(CombatAudioSpecs);
    return AllSpecs;
}

FAudio_PlaceholderSpec UAudio_PlaceholderLibrary::GetAudioSpecByID(const FString& AudioID) const
{
    TArray<FAudio_PlaceholderSpec> AllSpecs = GetAllAudioSpecs();
    
    for (const FAudio_PlaceholderSpec& Spec : AllSpecs)
    {
        if (Spec.AudioID == AudioID)
        {
            return Spec;
        }
    }
    
    return FAudio_PlaceholderSpec(); // Return default if not found
}

TArray<FAudio_PlaceholderSpec> UAudio_PlaceholderLibrary::GetAudioSpecsByCategory(const FString& Category) const
{
    TArray<FAudio_PlaceholderSpec> FilteredSpecs;
    TArray<FAudio_PlaceholderSpec> AllSpecs = GetAllAudioSpecs();
    
    for (const FAudio_PlaceholderSpec& Spec : AllSpecs)
    {
        if (Spec.AudioCategory == Category)
        {
            FilteredSpecs.Add(Spec);
        }
    }
    
    return FilteredSpecs;
}